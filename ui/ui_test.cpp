#include "ui.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

using namespace sdr;

int main() {
    auto wnd = ui::Window::create("Test", 800, 600);

    if (!wnd) {
        std::cerr << "Fatal error: cannot create UI window" << std::endl;
        return -1;
    }

    int avgCount = 5;
    float scale = 50.0f;

    std::ostringstream formatter;

    std::string fps = "FPS: 0";
    std::size_t frames = 0;
    double lastTime = 0;

    bool mpressed = false;
    bool prevShowLine = false;

    while (!wnd->closed()) {
        auto time = glfwGetTime();
        ++frames;

        if (time - lastTime >= 0.5) {
            formatter.str("");
            formatter << "FPS: " << frames * 2;
            fps = formatter.str();
            frames = 0;
            lastTime += 0.5;
        }

        double mx, my;
        glfwGetCursorPos(wnd->handle(), &mx, &my);
        auto wSize = wnd->size();

        bool showLine = (mx > 0 && mx < wSize.first &&
                         my > 0 && my < wSize.second &&
                         wnd->focused());

        wnd->update([&formatter,avgCount,scale,fps,mx,my,showLine](NVGcontext* vg, int width, int height) {
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

                formatter.str("");
                formatter << mx;

                nvgFillColor(vg, nvgRGBf(1.0f, 1.0f, 1.0f));
                nvgTextAlign(vg, ((width - mx < 100) ? NVG_ALIGN_RIGHT : NVG_ALIGN_LEFT) | NVG_ALIGN_MIDDLE);
                nvgText(vg, mx + ((width - mx < 100) ? -10 : 10), my, formatter.str().c_str(), NULL);
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
