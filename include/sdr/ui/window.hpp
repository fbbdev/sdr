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

#pragma once

#include "glad.h"
#include "nanovg.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "nuklear.h"

#include <functional>
#include <memory>

typedef struct GLFWwindow GLFWwindow;
typedef struct nk_glfw3_context nk_glfw3_context;

namespace sdr { namespace ui
{

class Window {
public:
    enum CursorMode {
        Normal,
        Hide,
        Grab
    };

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

    bool mouse_over() const {
        return mouse_over_;
    }

    bool closed() const;

    CursorMode cursor_mode() const;
    void cursor_mode(CursorMode mode);

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
    Window(GLFWwindow* wnd_, NVGcontext* vg_, nk_glfw3_context* gctx_);

    static void focus_callback(GLFWwindow* wnd, int state);
    static void minimize_callback(GLFWwindow* wnd, int state);
    static void mouse_over_callback(GLFWwindow* wnd, int state);

    GLFWwindow* wnd;
    NVGcontext* vg;
    nk_glfw3_context* gctx;
    nk_context* ctx;

    bool focused_ = false;
    bool minimized_ = false;
    bool mouse_over_ = false;
};

} /* namespace ui */ } /* namespace sdr */
