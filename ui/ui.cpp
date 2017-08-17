#include <cstdlib>
#include <cstring>

#include "ui.hpp"

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"

#include "nuklear/nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "font.hpp"

namespace
{
    bool glfw_initialized = false;
    bool glad_initialized = false;

    nk_color* style() {
        static nk_color table[NK_COLOR_COUNT];

        static const nk_color fg = nk_rgba(210, 210, 210, 255),
                              bg = nk_rgba(40, 40, 40, 255),
                              highlight = nk_rgba(80, 80, 80, 255),
                              window = nk_rgba(70, 70, 70, 200),
                              header = nk_rgba(50, 50, 50, 220),
                              accent = nk_rgba(58, 93, 240, 255),
                              accent_highlight = nk_rgba(78, 113, 240, 255),
                              accent_active = nk_rgba(48, 83, 200, 255);

        table[NK_COLOR_TEXT] = fg;
        table[NK_COLOR_WINDOW] = window;
        table[NK_COLOR_HEADER] = header;
        table[NK_COLOR_BORDER] = bg;
        table[NK_COLOR_BUTTON] = accent;
        table[NK_COLOR_BUTTON_HOVER] = accent_highlight;
        table[NK_COLOR_BUTTON_ACTIVE] = accent_active;
        table[NK_COLOR_TOGGLE] = bg;
        table[NK_COLOR_TOGGLE_HOVER] = highlight;
        table[NK_COLOR_TOGGLE_CURSOR] = accent;
        table[NK_COLOR_SELECT] = bg;
        table[NK_COLOR_SELECT_ACTIVE] = accent;
        table[NK_COLOR_SLIDER] = bg;
        table[NK_COLOR_SLIDER_CURSOR] = accent;
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = accent_highlight;
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = accent_active;
        table[NK_COLOR_PROPERTY] = bg;
        table[NK_COLOR_EDIT] = bg;
        table[NK_COLOR_EDIT_CURSOR] = fg;
        table[NK_COLOR_COMBO] = bg;
        table[NK_COLOR_CHART] = bg;
        table[NK_COLOR_CHART_COLOR] = accent;
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = fg;
        table[NK_COLOR_SCROLLBAR] = bg;
        table[NK_COLOR_SCROLLBAR_CURSOR] = accent;
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = accent_highlight;
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = accent_active;
        table[NK_COLOR_TAB_HEADER] = accent;

        return table;
    }
} /* namespace */

using namespace sdr;
using namespace ui;

std::unique_ptr<Window> Window::create(char const* title, int width, int height) {
    if (!glfw_initialized) {
        if (!(glfw_initialized = glfwInit()))
            return nullptr;

        std::atexit(glfwTerminate);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_SAMPLES, 4);

    auto wnd = glfwCreateWindow(width, height, title, nullptr, nullptr);

    glfwMakeContextCurrent(wnd);
    glfwSwapInterval(1);

    if (!glad_initialized) {
        if (!(glad_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))) {
            glfwDestroyWindow(wnd);
            return nullptr;
        }
    }

    auto vg = nvgCreateGL3(NVG_STENCIL_STROKES);
    if (!vg) {
        glfwDestroyWindow(wnd);
        return nullptr;
    }

    auto ctx = nk_glfw3_init(wnd, NK_GLFW3_INSTALL_CALLBACKS);
    if (!ctx) {
        nvgDeleteGL3(vg);
        glfwDestroyWindow(wnd);
        return nullptr;
    }

    return std::unique_ptr<Window>(new Window(wnd, vg, ctx));
}

Window::Window(GLFWwindow* wnd_, NVGcontext* vg_, nk_context* ctx_)
    : wnd(wnd_), vg(vg_), ctx(ctx_)
{
    glfwSetWindowUserPointer(wnd, this);

    glfwSetWindowFocusCallback(wnd, focus_callback);
    glfwSetWindowIconifyCallback(wnd, minimize_callback);

    int nvg_font = nvgCreateFontMem(vg, "default",
        const_cast<std::uint8_t*>(font_data), sizeof(font_data), false);
    nvgFontFaceId(vg, nvg_font);

    nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_font *nk_font = nk_font_atlas_add_from_memory(
        atlas, const_cast<std::uint8_t*>(font_data), sizeof(font_data), 18, nullptr);
    nk_glfw3_font_stash_end();
    nk_style_set_font(ctx, &nk_font->handle);

    nk_style_from_table(ctx, style());
    ctx->style.window.spacing = { 4, 8 };
    ctx->style.window.padding = { 4, 8 };
}

Window::~Window() {
    nk_glfw3_shutdown();
    nvgDeleteGL3(vg);
    glfwDestroyWindow(wnd);
}

std::pair<int, int> Window::size() const {
    std::pair<int, int> result;
    glfwGetWindowSize(wnd, &result.first, &result.second);
    return result;
}

std::pair<int, int> Window::fb_size() const {
    std::pair<int, int> result;
    glfwGetFramebufferSize(wnd, &result.first, &result.second);
    return result;
}

bool Window::closed() const {
    return glfwWindowShouldClose(wnd);
}

void Window::update(nk_color background,
                    std::function<void(int, int, int, int)> draw_gl,
                    std::function<void(NVGcontext*, int, int)> draw_vg,
                    std::function<void(nk_context*, int, int)> gui)
{
    glfwPollEvents();
    nk_glfw3_new_frame();

    int wWidth, wHeight,
        fbWidth, fbHeight;

    std::tie(wWidth, wHeight) = size();
    std::tie(fbWidth, fbHeight) = fb_size();

    if (gui)
        gui(ctx, wWidth, wHeight);

    float bg[4];
    nk_color_fv(bg, background);

    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(bg[0], bg[1], bg[2], bg[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);

    if (draw_gl)
        draw_gl(wWidth, wHeight, fbWidth, fbHeight);

    if (draw_vg) {
        //glDisable(GL_MULTISAMPLE);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        nvgBeginFrame(vg, wWidth, wHeight, float(fbWidth) / float(wWidth));
        draw_vg(vg, wWidth, wHeight);
        nvgEndFrame(vg);
        //glEnable(GL_MULTISAMPLE);
    }

    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(wnd);
}

void Window::focus_callback(GLFWwindow* wnd, int state) {
    auto cls = reinterpret_cast<Window*>(glfwGetWindowUserPointer(wnd));
    cls->focused_ = (state == GLFW_TRUE);
}

void Window::minimize_callback(GLFWwindow* wnd, int state) {
    auto cls = reinterpret_cast<Window*>(glfwGetWindowUserPointer(wnd));
    cls->minimized_ = (state == GLFW_TRUE);
}
