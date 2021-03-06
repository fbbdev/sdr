/**
 * sdr - software-defined radio building blocks for unix pipes
 * Copyright (C) 2017 Fabio Massaioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <cstring>

#include "window.hpp"

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#include "nuklear_glfw_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "resources/font.hpp"

namespace
{
    bool glfw_initialized = false;
    bool glad_initialized = false;

    nk_color* style() {
        static nk_color table[NK_COLOR_COUNT];

        static const nk_color fg = nk_rgba(210, 210, 210, 255),
                              bg = nk_rgba(40, 40, 40, 255),
                              highlight = nk_rgba(80, 80, 80, 255),
                              window = nk_rgba(70, 70, 70, 255),
                              header = nk_rgba(50, 50, 50, 255),
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

using namespace sdr::ui;

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

    auto gctx = nk_glfw3_init(wnd, NK_GLFW3_INSTALL_CALLBACKS);
    if (!gctx) {
        nvgDeleteGL3(vg);
        glfwDestroyWindow(wnd);
        return nullptr;
    }

    return std::unique_ptr<Window>(new Window(wnd, vg, gctx));
}

Window::Window(GLFWwindow* wnd_, NVGcontext* vgc_, nk_glfw3_context* gctx_)
    : wnd(wnd_), vg_(vgc_), gctx(gctx_), ctx(gctx_->ctx)
{
    gctx->userdata = reinterpret_cast<void*>(this);

    glfwSetWindowFocusCallback(wnd, focus_callback);
    glfwSetWindowIconifyCallback(wnd, minimize_callback);
    glfwSetCursorEnterCallback(wnd, mouse_over_callback);

    int nvg_font = nvgCreateFontMem(vg_, "default",
        const_cast<std::uint8_t*>(font_data), sizeof(font_data), false);
    nvgFontFaceId(vg_, nvg_font);

    nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(gctx, &atlas);
    nk_font *nk_font = nk_font_atlas_add_from_memory(
        atlas, const_cast<std::uint8_t*>(font_data), sizeof(font_data), 18, nullptr);
    nk_glfw3_font_stash_end(gctx);
    nk_style_set_font(ctx, &nk_font->handle);

    nk_style_from_table(ctx, style());
    ctx->style.window.spacing = { 4, 8 };
    ctx->style.window.padding = { 4, 8 };
}

Window::~Window() {
    nk_glfw3_shutdown(gctx);
    nvgDeleteGL3(vg_);
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

Window::CursorMode Window::cursor_mode() const {
    int m = glfwGetInputMode(wnd, GLFW_CURSOR);
    return (m == GLFW_CURSOR_DISABLED) ? CursorMode::Grab :
           (m == GLFW_CURSOR_HIDDEN)   ? CursorMode::Hide
                                       : CursorMode::Normal;
}

void Window::cursor_mode(CursorMode mode) {
    int m = (mode == CursorMode::Grab) ? GLFW_CURSOR_DISABLED :
            (mode == CursorMode::Hide) ? GLFW_CURSOR_HIDDEN
                                       : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(wnd, GLFW_CURSOR, m);
}

void Window::update(NVGcolor background,
                    std::function<void(int, int, int, int)> draw_gl,
                    std::function<void(NVGcontext*, int, int)> draw_vg,
                    std::function<void(Window*, int, int)> gui)
{
    if (ctx->input.mouse.grab && !ctx->input.mouse.grabbed) {
        ctx->input.mouse.grabbed = true;
        cursor_mode(ui::Window::CursorMode::Grab);
    }
    if (ctx->input.mouse.ungrab && ctx->input.mouse.grabbed) {
        ctx->input.mouse.grabbed = false;
        cursor_mode(ui::Window::CursorMode::Normal);
    }

    glfwPollEvents();
    nk_glfw3_new_frame(gctx);

    int wWidth, wHeight,
        fbWidth, fbHeight;

    std::tie(wWidth, wHeight) = size();
    std::tie(fbWidth, fbHeight) = fb_size();

    if (gui)
        gui(this, wWidth, wHeight);

    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(background.r, background.g, background.b, background.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);

    if (draw_gl)
        draw_gl(wWidth, wHeight, fbWidth, fbHeight);

    if (draw_vg) {
        //glDisable(GL_MULTISAMPLE);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        nvgBeginFrame(vg_, wWidth, wHeight, float(fbWidth) / float(wWidth));
        draw_vg(vg_, wWidth, wHeight);
        nvgEndFrame(vg_);
        //glEnable(GL_MULTISAMPLE);
    }

    nk_glfw3_render(gctx, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    glfwSwapBuffers(wnd);
}

void Window::focus_callback(GLFWwindow* wnd, int state) {
    auto cls = reinterpret_cast<Window*>(
        reinterpret_cast<nk_glfw3_context*>(glfwGetWindowUserPointer(wnd))->userdata);
    cls->focused_ = !!state;
}

void Window::minimize_callback(GLFWwindow* wnd, int state) {
    auto cls = reinterpret_cast<Window*>(
        reinterpret_cast<nk_glfw3_context*>(glfwGetWindowUserPointer(wnd))->userdata);
    cls->minimized_ = !!state;
}

void Window::mouse_over_callback(GLFWwindow* wnd, int state) {
    auto cls = reinterpret_cast<Window*>(
        reinterpret_cast<nk_glfw3_context*>(glfwGetWindowUserPointer(wnd))->userdata);
    cls->mouse_over_ = !!state;
}
