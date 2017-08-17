#pragma once

#include <functional>
#include <memory>

#include "glad/glad.h"

#include "nanovg/nanovg.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "nuklear/nuklear.h"

typedef struct GLFWwindow GLFWwindow;

namespace sdr { namespace ui
{

class Window {
public:
    static std::unique_ptr<Window> create(char const* title, int width, int height);

    ~Window();

    GLFWwindow* handle() const {
        return wnd;
    }

    NVGcontext* vg_context() const {
        return vg;
    }

    nk_context* gui_context() const {
        return ctx;
    }

    std::pair<int, int> size() const;

    std::pair<int, int> fb_size() const;

    bool focused() const {
        return focused_;
    }

    bool minimized() const {
        return minimized_;
    }

    bool closed() const;

    void update(nk_color background,
                std::function<void(int, int, int, int)> draw_gl,
                std::function<void(NVGcontext*, int, int)> draw_vg,
                std::function<void(nk_context*, int, int)> gui);

    void update(std::function<void(int, int, int, int)> draw_gl,
                std::function<void(NVGcontext*, int, int)> draw_vg,
                std::function<void(nk_context*, int, int)> gui) {
        update(nk_rgb(0, 0, 0), draw_gl, draw_vg, gui);
    }

    void update(nk_color background,
                std::function<void(nk_context*, int, int)> gui) {
        update(background, nullptr, nullptr, gui);
    }

    void update(std::function<void(nk_context*, int, int)> gui) {
        update(nk_rgb(0, 0, 0), nullptr, nullptr, gui);
    }

    void update(nk_color background,
                std::function<void(NVGcontext*, int, int)> draw_vg,
                std::function<void(nk_context*, int, int)> gui) {
        update(background, nullptr, draw_vg, gui);
    }

    void update(std::function<void(NVGcontext*, int, int)> draw_vg,
                std::function<void(nk_context*, int, int)> gui) {
        update(nk_rgb(0, 0, 0), nullptr, draw_vg, gui);
    }
    
private:
    Window(GLFWwindow* wnd_, NVGcontext* vg_, nk_context* ctx_);

    static void focus_callback(GLFWwindow* wnd, int state);
    static void minimize_callback(GLFWwindow* wnd, int state);

    GLFWwindow* wnd;
    NVGcontext* vg;
    nk_context* ctx;

    bool focused_ = false;
    bool minimized_ = false;
};

} /* namespace ui */ } /* namespace sdr */
