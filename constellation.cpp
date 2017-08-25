#include "stream/stream.hpp"
#include "ui/ui.hpp"
#include "ui/view.hpp"

#include "opt/opt.hpp"

#include <cmath>
#include <algorithm>
#include <chrono>
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

void processor(std::uint16_t id, bool tap, bool throttle) {
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

        auto duration = source->packet().duration;
        auto pkt_size = source->packet().count<std::complex<float>>();
        auto size = pkt_size;

        while (size && !source->end()) {
            buffer_lock.lock();
            auto read = source->recv(&*it, std::min(size, std::uint32_t(buf.end() - it)));
            buffer_lock.unlock();

            size -= read;
            it += read;

            if (it == buf.end())
                it = buf.begin();

            if (throttle && duration)
                std::this_thread::sleep_for(
                        std::chrono::nanoseconds(duration*read/pkt_size));
        }
    }
}

int main(int argc, char* argv[]) {
    using opt::Option;
    using opt::Placeholder;

    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<std::uintmax_t> points("points", Placeholder("POINTS"), 1000);
    Option<bool> tap("tap", false);
    Option<bool> throttle("throttle", false);

    if (!opt::parse({ id }, { points, tap, throttle }, argv, argv + argc))
        return -1;

    auto wnd = ui::Window::create("Constellation", 300, 300);

    if (!wnd) {
        std::cerr << "Fatal error: cannot create window" << std::endl;
        return -1;
    }

    buf.resize(points);
    auto local_buf = buf;

    std::thread proc(processor, id, tap, throttle);
    proc.detach();

    ui::View view(ui::View::IsometricFitMin, { 4.0f, -4.0f });
    struct nk_vec2 mouse = { 0.0f, 0.0f };

    while (!wnd->closed()) {
        buffer_lock.lock();
        std::copy(buf.begin(), buf.end(), local_buf.begin());
        buffer_lock.unlock();

        auto focused = wnd->focused();

        wnd->update(nk_rgb_f(0.05, 0.07, 0.05), [&local_buf,&v=view,mouse,focused](NVGcontext* vg, int width, int height) {
            std::ostringstream fmt;
            fmt << std::defaultfloat << std::setprecision(3);

            bool showCursor = focused && (mouse.x > 0 && mouse.x < width)
                                      && (mouse.y > 0 && mouse.y < height);

            auto view = v.compute(width, height);
            auto center = view.global({ 0, 0 });
            // draw grid

            // draw center lines
            nvgFillColor(vg, nvgRGB(60, 180, 60));
            nvgStrokeColor(vg, nvgRGB(60, 180, 60));

            nvgBeginPath(vg);
            if (center.x > 0 && center.x < width) {
                nvgMoveTo(vg, center.x, 0);
                nvgLineTo(vg, center.x, height);

                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgText(vg, center.x + 5, 5, "0", 0);
            }

            if (center.y > 0 && center.y < height) {
                nvgMoveTo(vg, 0, center.y);
                nvgLineTo(vg, width, center.y);

                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
                nvgText(vg, 5, center.y - 5, "0", 0);
            }
            nvgStroke(vg);

            // draw lines every ~100px
            auto grid_step = std::pow(2.0f, std::round(std::log2(view.local_delta_x(100))));

            nvgFillColor(vg, nvgRGBA(50, 100, 50, 230));
            nvgStrokeColor(vg, nvgRGBA(50, 100, 50, 230));

            nvgBeginPath(vg);

            // vertical lines
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

            auto grid_start_x =
                grid_step * std::floor(view.local_x(0)/grid_step);
            auto grid_end_x =
                grid_step * (1 + std::ceil(view.local_x(width)/grid_step));

            for (float x = grid_start_x; x < grid_end_x; x += grid_step) {
                if (x > -grid_step && x < grid_step)
                    // skip 0
                    continue;

                auto gx = view.global_x(x);
                nvgMoveTo(vg, gx, 0);
                nvgLineTo(vg, gx, height);

                fmt.str(std::string());
                fmt << x;
                nvgText(vg, gx + 5, 5, fmt.str().c_str(), nullptr);
            }

            // horizontal lines
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);

            auto grid_start_y =
                grid_step * std::floor(view.local_y(height)/grid_step);
            auto grid_end_y =
                grid_step * (1 + std::ceil(view.local_y(0)/grid_step));

            for (float y = grid_start_y; y < grid_end_y; y += grid_step) {
                if (y > -grid_step && y < grid_step)
                    // skip 0
                    continue;

                auto gy = view.global_y(y);
                nvgMoveTo(vg, 0, gy);
                nvgLineTo(vg, width, gy);

                fmt.str(std::string());
                fmt << y;
                nvgText(vg, 5, gy - 5, fmt.str().c_str(), nullptr);
            }

            nvgStroke(vg);

            // draw half step lines
            nvgStrokeColor(vg, nvgRGBA(30, 80, 30, 200));
            nvgStrokeWidth(vg, 0.6f);

            nvgBeginPath(vg);

            // vertical lines
            for (float x = grid_start_x + (grid_step/2); x < grid_end_x; x += grid_step) {
                auto gx = view.global_x(x);
                nvgMoveTo(vg, gx, 0);
                nvgLineTo(vg, gx, height);
            }

            // horizontal lines
            for (float y = grid_start_y + (grid_step/2); y < grid_end_y; y += grid_step) {
                auto gy = view.global_y(y);
                nvgMoveTo(vg, 0, gy);
                nvgLineTo(vg, width, gy);
            }

            nvgStroke(vg);
            nvgStrokeWidth(vg, 1);

            // draw constellation
            view.apply(vg);

            nvgFillColor(vg, nvgRGB(80, 208, 80));
            nvgBeginPath(vg);
            auto r = view.local_delta_x(1.5f);
            for (auto sample: local_buf) {
                nvgCircle(vg, sample.real(), sample.imag(), r);
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

                auto coord = view.local(mouse);

                fmt.str(std::string());
                fmt << coord.x << ((coord.y < 0) ? '-' : '+') << 'j' << std::abs(coord.y);

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
        }, [&view,&mouse](nk_context* ctx, int width, int height) {
            view.zoom(ctx->input.mouse.scroll_delta.y);

            mouse = ctx->input.mouse.pos;

            if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT))
                view.move(view.compute(width, height)
                              .local_delta(ctx->input.mouse.delta));
        });
    }

    return 0;
}
