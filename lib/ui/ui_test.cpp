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

    ui::Plate plate;

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

        wnd->update([avgCount,scale,fps,&plate,mx,my,showLine](NVGcontext* vg, int width, int height) {
            nvgFillColor(vg, nvgRGBf(1.0f, 1.0f, 1.0f));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgText(vg, 10, 10, fps.c_str(), NULL);

            nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.1f*avgCount));
            nvgBeginPath(vg);
            nvgCircle(vg, width/2.0f, height/2.0f, scale);
            nvgFill(vg);

            if (showLine) {
                nvgStrokeColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
                nvgBeginPath(vg);
                nvgMoveTo(vg, mx, 0);
                nvgLineTo(vg, mx, height);
                nvgStroke(vg);

                plate.draw(vg, ui::format(mx),
                           ((width - mx < 100) ? NVG_ALIGN_RIGHT : NVG_ALIGN_LEFT) | NVG_ALIGN_MIDDLE,
                           { float(mx), float(my) });
            }
        }, [&avgCount,&scale,mx,prevShowLine,showLine,&mpressed](nk_context* ctx, int width, int height) {
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


            if (nk_begin(ctx, "Settings", nk_rect(width - w - 10, y, w, h),
                         NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_NO_SCROLLBAR))
            {
                nk_layout_row_dynamic(ctx, 25, 1);

                nk_property_int(ctx, "Average count:", 1, &avgCount, std::numeric_limits<int>::max(), 1, 0.25);
                nk_property_float(ctx, "Scale:", 0.0f, &scale, std::numeric_limits<float>::max(), 5, 1.0f);

                if (nk_button_label(ctx, "Autoscale"))
                    std::cout << "autoscale" << std::endl;
            }
            nk_end(ctx);
        });

        prevShowLine = showLine;
    }

    return 0;
}
