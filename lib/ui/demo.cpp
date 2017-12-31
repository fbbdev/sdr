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

#include "ui/ui.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <limits>

using namespace sdr;

int main() {
    auto wnd = ui::Window::create("Test", 800, 600);

    if (!wnd) {
        std::cerr << "Fatal error: cannot create UI window" << std::endl;
        return -1;
    }

    int avgCount = 5;
    float scale = 50.0f;

    std::string fps = "FPS: 0";
    std::size_t frames = 0;
    double lastTime = 0;

    bool mpressed = false;
    bool prevShowLine = false;

    ui::Cursor cursor(ui::Cursor::Vertical, ui::Plate(NVG_ALIGN_MIDDLE));

    bool special_style = false;
    auto window_style_normal = wnd->gui()->style.window;
    auto window_style_special = window_style_normal;

    window_style_special.background = nk_rgba(0, 0, 0, 0);
    window_style_special.fixed_background = nk_style_item_hide();
    window_style_special.header.normal = nk_style_item_hide();
    window_style_special.header.hover = nk_style_item_hide();
    window_style_special.header.active = nk_style_item_hide();
    window_style_special.header.minimize_button.normal = nk_style_item_color(nk_rgba(255, 255, 255, 20));
    window_style_special.header.minimize_button.hover = nk_style_item_color(nk_rgba(255, 255, 255, 40));
    window_style_special.header.minimize_button.active = nk_style_item_color(nk_rgba(100, 100, 100, 100));
    window_style_special.header.minimize_button.rounding = 12;
    window_style_special.header.minimize_button.text_alignment = NK_TEXT_ALIGN_CENTERED;
    window_style_special.header.minimize_button.padding = { -1, 0 };

    while (!wnd->closed()) {
        auto time = glfwGetTime();
        ++frames;

        if (time - lastTime >= 0.5) {
            fps = "FPS: " + ui::format(frames * 2);
            frames = 0;
            lastTime += 0.5;
        }

        double mx, my;
        glfwGetCursorPos(wnd->handle(), &mx, &my);

        bool showLine = wnd->focused() && wnd->mouse_over();

        wnd->update([avgCount,scale,fps,&cursor,mx,my,showLine](NVGcontext* vg, int width, int height) {
            nvgFillColor(vg, nvgRGBf(1.0f, 1.0f, 1.0f));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgText(vg, 10, 10, fps.c_str(), NULL);

            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.1f*avgCount));
            nvgBeginPath(vg);
            nvgCircle(vg, width/2.0f, height/2.0f, scale);
            nvgFill(vg);

            if (showLine) {
                cursor.draw(vg, { 0.0f, 0.0f, float(width), float(height) },
                            { float(mx), float(my) }, ui::format(mx));
            }
        }, [&avgCount,&scale,mx,prevShowLine,showLine,&mpressed,
            &special_style,&window_style_normal,&window_style_special](ui::Window* wnd, int width, int height) {
            auto ctx = wnd->gui();

            if (prevShowLine && showLine) {
                // Accept click only when press and release both happened
                // on the background
                if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT))
                    mpressed = !nk_item_is_any_active(ctx);
                else if (nk_input_is_mouse_released(&ctx->input, NK_BUTTON_LEFT)) {
                    if (mpressed && !nk_item_is_any_active(ctx))
                        std::cout << "background click at " << mx << std::endl;
                    mpressed = false;
                }
            }

            int header_height =
                ctx->style.font->height +
                2 * ctx->style.window.header.padding.y +
                2 * ctx->style.window.header.label_padding.y;

            auto w = std::min(180, width - 20),
                 h = std::min(107 + header_height, height - 20);

            auto y = height - (nk_window_is_collapsed(ctx, "Settings") ? header_height : h) - 10;

            if (special_style)
                ctx->style.window = window_style_special;
            int p = nk_begin(ctx, "Settings", nk_rect(width - w - 10, y, w, h),
                             NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_NO_SCROLLBAR);
            ctx->style.window = window_style_normal;

            if (p) {
                nk_layout_row_dynamic(ctx, 25, 1);

                nk_property_int(ctx, "Average count:", 1, &avgCount, std::numeric_limits<int>::max(), 1, 0.25);
                nk_property_float(ctx, "Scale:", 0.0f, &scale, std::numeric_limits<float>::max(), 5, 1.0f);

                if (nk_button_label(ctx, "Autoscale")) {
                    special_style = !special_style;
                    std::cout << (special_style ? "special" : "normal")
                              << " style" << std::endl;
                }
            }
            nk_end(ctx);
        });

        prevShowLine = showLine;
    }

    return 0;
}
