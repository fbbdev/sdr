#include "ui/ui.hpp"
#include "stream/stream.hpp"

#include <cmath>
#include <algorithm>
#include <complex>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace sdr;

std::mutex buffer_lock;
std::vector<std::complex<float>> buf;

void processor(std::uint16_t id, bool tap) {
    auto it = buf.begin();

    auto source = stdin_source();
    auto sink = stdout_sink();

    while (source->next()) {
        if (source->packet().id != id || source->packet().content != Packet::ComplexTimeSignal) {
            if (tap)
                source->pass(sink);
            else
                source->drop();

            continue;
        }

        if (tap)
            source->copy(sink);

        auto size = source->packet().count<std::complex<float>>();

        while (size && !source->end()) {
            buffer_lock.lock();
            auto read = source->recv(&*it, std::min(size, std::uint32_t(buf.end() - it)));
            buffer_lock.unlock();

            size -= read;
            it += read;

            if (it == buf.end())
                it = buf.begin();
        }
    }
}

int main() {
    auto wnd = ui::Window::create("Constellation", 300, 300);

    if (!wnd) {
        std::cerr << "Fatal error: cannot create window" << std::endl;
        return -1;
    }

    std::uint16_t id = 0;
    unsigned int points = 1000;
    bool tap = false;

    buf.resize(points);
    auto local_buf = buf;

    std::thread proc(processor, id, tap);
    proc.detach();

    float zoom = 0.5f;
    struct nk_vec2 pos = { 0.0f, 0.0f };
    struct nk_vec2 mouse = { 0.0f, 0.0f };

    while (!wnd->closed()) {
        buffer_lock.lock();
        std::copy(buf.begin(), buf.end(), local_buf.begin());
        buffer_lock.unlock();

        auto focused = wnd->focused();

        wnd->update(nk_rgb_f(0.05, 0.07, 0.05), [&local_buf,zoom,pos,mouse,focused](NVGcontext* vg, int width, int height) {
            std::ostringstream fmt;
            fmt << std::defaultfloat << std::setprecision(3);

            bool showCursor = focused && (mouse.x > 0 && mouse.x < width)
                                      && (mouse.y > 0 && mouse.y < height);

            auto center_x = width/2.0f + pos.x;
            auto center_y = height/2.0f + pos.y;

            auto scale = zoom*std::min(width, height)/2.0f;

            // draw grid

            // draw center lines
            nvgFillColor(vg, nvgRGB(60, 180, 60));
            nvgStrokeColor(vg, nvgRGB(60, 180, 60));
            if (center_x > 0 && center_x < width) {
                nvgBeginPath(vg);
                nvgMoveTo(vg, center_x, 0);
                nvgLineTo(vg, center_x, height);
                nvgStroke(vg);

                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgText(vg, center_x + 5, 5, "0", 0);
            }

            if (center_y > 0 && center_y < height) {
                nvgBeginPath(vg);
                nvgMoveTo(vg, 0, center_y);
                nvgLineTo(vg, width, center_y);
                nvgStroke(vg);

                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
                nvgText(vg, 5, center_y - 5, "0", 0);
            }

            // draw lines every ~100px
            auto grid_step = 1.0f / std::pow(2.0f, std::round(std::log2(scale/100)));

            nvgFillColor(vg, nvgRGBA(50, 100, 50, 230));
            nvgStrokeColor(vg, nvgRGBA(50, 100, 50, 230));

            nvgBeginPath(vg);

            // vertical lines
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

            auto grid_start_x =
                grid_step * std::floor(-center_x/scale/grid_step);
            auto grid_end_x =
                grid_step * (1 + std::ceil((width-center_x)/scale/grid_step));

            for (float x = grid_start_x; x < grid_end_x; x += grid_step) {
                if (x > -grid_step && x < grid_step)
                    // skip 0
                    continue;

                nvgMoveTo(vg, x*scale + center_x, 0);
                nvgLineTo(vg, x*scale + center_x, height);

                fmt.str(std::string());
                fmt << x;
                nvgText(vg, x*scale + center_x + 5, 5, fmt.str().c_str(), nullptr);
            }

            // horizontal lines
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);

            auto grid_start_y =
                grid_step * std::floor((height-center_y)/-scale/grid_step);
            auto grid_end_y =
                grid_step * (1 + std::ceil(-center_y/-scale/grid_step));

            for (float y = grid_start_y; y < grid_end_y; y += grid_step) {
                if (y > -grid_step && y < grid_step)
                    // skip 0
                    continue;

                nvgMoveTo(vg, 0, -y*scale + center_y);
                nvgLineTo(vg, width, -y*scale + center_y);

                fmt.str(std::string());
                fmt << y;
                nvgText(vg, 5, -y*scale + center_y - 5, fmt.str().c_str(), nullptr);
            }

            nvgStroke(vg);

            // draw half step lines
            nvgStrokeColor(vg, nvgRGBA(30, 80, 30, 200));
            nvgStrokeWidth(vg, 0.6f);

            nvgBeginPath(vg);

            // vertical lines
            for (float x = grid_start_x + (grid_step/2); x < grid_end_x; x += grid_step) {
                nvgMoveTo(vg, x*scale + center_x, 0);
                nvgLineTo(vg, x*scale + center_x, height);
            }

            // horizontal lines
            for (float y = grid_start_y + (grid_step/2); y < grid_end_y; y += grid_step) {
                nvgMoveTo(vg, 0, -y*scale + center_y);
                nvgLineTo(vg, width, -y*scale + center_y);
            }

            nvgStroke(vg);
            nvgStrokeWidth(vg, 1);

            // draw constellation
            nvgTranslate(vg, center_x, center_y);
            nvgScale(vg, scale, -scale);

            nvgFillColor(vg, nvgRGB(80, 208, 80));
            nvgBeginPath(vg);
            for (auto sample: local_buf) {
                nvgCircle(vg, sample.real(), sample.imag(), 1.5f/scale);
            }
            nvgFill(vg);

            // draw cursor
            if (showCursor) {
                nvgResetTransform(vg);

                nvgStrokeColor(vg, nvgRGBf(1.0f, 1.0f, 1.0f));

                nvgBeginPath(vg);
                nvgMoveTo(vg, mouse.x, 0);
                nvgLineTo(vg, mouse.x, height);
                nvgMoveTo(vg, 0, mouse.y);
                nvgLineTo(vg, width, mouse.y);
                nvgStroke(vg);

                auto coord_x = (mouse.x - center_x)/scale,
                     coord_y = -(mouse.y - center_y)/scale;

                fmt.str(std::string());
                fmt << coord_x << ((coord_y < 0) ? '-' : '+') << 'j' << std::abs(coord_y);

                bool left   = (width - mouse.x < 100),
                     bottom = (mouse.y < 50);

                nvgTextAlign(vg, (left ? NVG_ALIGN_RIGHT : NVG_ALIGN_LEFT) |
                                 (bottom ? NVG_ALIGN_TOP : NVG_ALIGN_BOTTOM));

                float bounds[4];
                nvgTextBounds(vg,
                    mouse.x + (left ? -10 : 10), mouse.y + (bottom ? 10 : -10),
                    fmt.str().c_str(), nullptr, bounds);
                float label_width = bounds[2] - bounds[0];
                float label_height = bounds[3] - bounds[1];

                // Text background, for readability
                nvgFillColor(vg, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));
                nvgBeginPath(vg);
                nvgRoundedRect(vg,
                    mouse.x + (left ? -15-label_width : 5), mouse.y + (bottom ? 5 : -15 - label_height),
                    label_width + 10, label_height + 10, 3);
                nvgFill(vg);

                nvgFillColor(vg, nvgRGBf(1.0f, 1.0f, 1.0f));
                nvgText(vg,
                    mouse.x + (left ? -10 : 10), mouse.y + (bottom ? 10 : -10),
                    fmt.str().c_str(), nullptr);
            }
        }, [&zoom,&pos,&mouse](nk_context* ctx, int, int) {
            auto delta = ctx->input.mouse.scroll_delta.y,
                 factor = std::pow(1.1f, delta);

            // Keep zoom in floating-point limits
            if ((delta > 0 && zoom < std::numeric_limits<float>::max()/factor) ||
                (delta < 0 && zoom > std::numeric_limits<float>::min()*factor)) {
                zoom *= factor;
                pos = nk_vec2_muls(pos, factor);
            }

            mouse = ctx->input.mouse.pos;

            if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT))
                pos = nk_vec2_add(pos, ctx->input.mouse.delta);
        });
    }

    return 0;
}
