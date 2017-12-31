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

#include "options.hpp"
#include "signal.hpp"
#include "stream.hpp"
#include "ui/ui.hpp"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

using namespace sdr;

std::mutex buffer_lock;
std::vector<Sample> buf;

void processor(std::uint16_t id, bool throttle) {
    auto it = buf.begin();

    Source source;
    Sink sink;

    auto next_packet = std::chrono::high_resolution_clock::now();
    while (source.next()) {
        if (source.packet().id != id || (source.packet().content != Packet::Signal &&
                                         source.packet().content != Packet::ComplexSignal)) {
            continue;
        }

        const auto duration = source.packet().duration;

        if (source.packet().content == Packet::Signal) {
            auto data = source.recv<RealSample>();
            auto data_it = data.begin(), data_end = data.end();

            while (data_it != data_end) {
                buffer_lock.lock();
                auto copied = std::copy_n(data_it, std::min(data_end - data_it, buf.end() - it), it) - it;
                buffer_lock.unlock();

                data_it += copied;
                it += copied;

                if (it == buf.end())
                    it = buf.begin();

                if (throttle && duration) {
                    next_packet += std::chrono::nanoseconds(duration*(data_it - data.begin())/data.size());
                    std::this_thread::sleep_until(next_packet);
                }
            }
        } else {
            const auto pkt_size = source.packet().count<Sample>();
            auto size = pkt_size;

            while (size && !source.end()) {
                buffer_lock.lock();
                auto read = source.recv(&*it, std::min(size, std::uint32_t(buf.end() - it)));
                buffer_lock.unlock();

                size -= read;
                it += read;

                if (it == buf.end())
                    it = buf.begin();

                if (throttle && duration) {
                    next_packet += std::chrono::nanoseconds(duration*(pkt_size - size)/pkt_size);
                    std::this_thread::sleep_until(next_packet);
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<std::uintmax_t> points("points", Placeholder("POINTS"), 1000);
    Option<bool> throttle("throttle", false);
    Option<std::string> title("title", "Constellation");

    if (!opt::parse({ id }, { points, throttle, title }, argv, argv + argc))
        return -1;

    if (!valid_stream_id(id.get())) {
        std::cerr << "error: gen: " << id.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    auto wnd = ui::Window::create(title.get().c_str(), 300, 300);

    if (!wnd) {
        std::cerr << "constellation: error: cannot create window" << std::endl;
        return -1;
    }

    buf.resize(points);
    auto local_buf = buf;

    std::thread proc(processor, id, throttle);
    proc.detach();

    ui::InteractiveView view(ui::View::IsometricFitMin, { 4.0f, -4.0f });
    struct nk_vec2 mouse = { 0.0f, 0.0f };

    ui::Grid grid({
        { { nvgRGB(60, 180, 60), 1.0f }, {
            ui::Scale(ui::Scale::Horizontal, 0, { -0.5, 0.5 }, 1.0f),
            ui::Scale(ui::Scale::Vertical, 0, { -0.5, 0.5 }, 1.0f),
        } },
        { { nvgRGBA(50, 100, 50, 230), 1.0f }, {
            ui::Scale(ui::Scale::Horizontal, 0, { -INFINITY, -0.5f }, 100, 1.0f),
            ui::Scale(ui::Scale::Horizontal, 0, { 0.5f, INFINITY }, 100, 1.0f),
            ui::Scale(ui::Scale::Vertical, 0, { -INFINITY, -0.5f }, 100, 1.0f),
            ui::Scale(ui::Scale::Vertical, 0, { 0.5f, INFINITY }, 100, 1.0f),
        } },
        { { nvgRGBA(30, 80, 30, 200), 0.6f, false }, {
            ui::Scale(ui::Scale::Horizontal, 0.5f, 100, 1.0f),
            ui::Scale(ui::Scale::Vertical, 0.5f, 100, 1.0f),
        } },
    });

    ui::Plate plate;

    while (!wnd->closed()) {
        buffer_lock.lock();
        std::copy(buf.begin(), buf.end(), local_buf.begin());
        buffer_lock.unlock();

        auto showCursor = wnd->focused() && wnd->mouse_over() &&
                          wnd->cursor_mode() != ui::Window::CursorMode::Grab;

        wnd->update(nvgRGBf(0.05, 0.07, 0.05), [&local_buf,&v=view,&plate,&grid,mouse,showCursor](NVGcontext* vg, int width, int height) {
            auto view = v.compute(width, height);

            grid.draw(vg, view);

            // draw constellation
            nvgSave(vg);

            view.apply(vg);

            nvgFillColor(vg, nvgRGB(80, 208, 80));
            nvgBeginPath(vg);
            auto r = view.local_delta_x(1.5f);
            for (auto sample: local_buf) {
                nvgCircle(vg, sample.real(), sample.imag(), r);
            }
            nvgFill(vg);

            nvgRestore(vg);

            // draw cursor
            if (showCursor) {
                // Lines
                nvgStrokeColor(vg, nvgRGBf(0.6f, 0.6f, 0.6f));

                nvgBeginPath(vg);
                nvgMoveTo(vg, mouse.x, 0);
                nvgLineTo(vg, mouse.x, height);
                nvgMoveTo(vg, 0, mouse.y);
                nvgLineTo(vg, width, mouse.y);
                nvgStroke(vg);

                auto coord = view.local(mouse);
                ui::Vec2 pixel_mag = {
                    std::floor(std::log10(std::abs(view.local_delta_x(1)))),
                    std::floor(std::log10(std::abs(view.local_delta_y(1))))
                };

                std::string label = ui::format(coord.x, pixel_mag.x) +
                                    ((coord.y < 0) ? "-" : "+") + "j" +
                                    ui::format(std::abs(coord.y), pixel_mag.y);

                bool left   = (width - mouse.x < 100),
                     bottom = (mouse.y < 50);

                int align = (left ? NVG_ALIGN_RIGHT : NVG_ALIGN_LEFT) |
                            (bottom ? NVG_ALIGN_TOP : NVG_ALIGN_BOTTOM);

                plate.draw(vg, label, align, mouse);
            }
        }, [&view,&mouse](ui::Window* w, int width, int height) {
            mouse = w->gui()->input.mouse.pos;
            view.interact(w, { 0, 0, float(width), float(height) });
        });
    }

    return 0;
}
