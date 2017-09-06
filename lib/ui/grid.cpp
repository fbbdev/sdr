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

#include "grid.hpp"

#include <iomanip>
#include <sstream>

using namespace sdr::ui;

static constexpr int halign_mask = NVG_ALIGN_LEFT |
                                   NVG_ALIGN_CENTER |
                                   NVG_ALIGN_RIGHT;
static constexpr int valign_mask = NVG_ALIGN_TOP |
                                   NVG_ALIGN_MIDDLE |
                                   NVG_ALIGN_RIGHT |
                                   NVG_ALIGN_BASELINE;

static constexpr int halign(int align) {
    int a = align & halign_mask;
    return a ? a : NVG_ALIGN_LEFT;
}

static constexpr int valign(int align) {
    int a = align & valign_mask;
    return a ? a : NVG_ALIGN_TOP;
}

static constexpr int valign_vert(int align) {
    int a = valign(align);

    if (a & NVG_ALIGN_TOP)
        return NVG_ALIGN_BOTTOM;
    else if (a & NVG_ALIGN_BOTTOM)
        return NVG_ALIGN_TOP;

    return a;
}

void Grid::draw(NVGcontext* vg, AppliedView const& view) const {
    std::ostringstream fmt;
    fmt << std::defaultfloat;

    nvgSave(vg);

    for (auto const& lst: scales_) {
        auto const& style = lst.first;

        nvgStrokeColor(vg, style.color);
        nvgFillColor(vg, style.color);

        nvgStrokeWidth(vg, style.stroke_width);

        for (auto const& s: lst.second) {
            auto scale = s.compute(view);

            if (scale.orientation() == Scale::Horizontal) {
                auto ha = halign(style.label_align);

                nvgTextAlign(vg, ha | valign(style.label_align));

                float label_x =
                    (ha & NVG_ALIGN_LEFT) ? style.label_distance :
                        (ha & NVG_ALIGN_RIGHT) ? -style.label_distance : 0.0f;
                float label_y = style.label_margin + (view.height - 2*style.label_margin)*style.label_position;

                nvgBeginPath(vg);

                for (auto x: scale) {
                    auto gx = view.global_x(x);
                    nvgMoveTo(vg, gx, 0);
                    nvgLineTo(vg, gx, view.height);

                    if (style.label) {
                        fmt.str(std::string());
                        fmt << std::setprecision(std::max(3l, long(std::floor(std::log10(std::abs(x))) - scale.step_magnitude()) + 1))
                            << x;
                        nvgText(vg, gx + label_x, label_y, fmt.str().c_str(), nullptr);
                    }
                }

                nvgStroke(vg);
            } else {
                auto va = valign_vert(style.label_align);

                nvgTextAlign(vg, halign(style.label_align) | va);

                float label_x = style.label_margin + (view.width - 2*style.label_margin)*style.label_position;
                float label_y =
                    (va & NVG_ALIGN_TOP) ? style.label_distance :
                        (va & NVG_ALIGN_BOTTOM) ? -style.label_distance : 0.0f;

                nvgBeginPath(vg);

                for (auto y: scale) {
                    auto gy = view.global_y(y);
                    nvgMoveTo(vg, 0, gy);
                    nvgLineTo(vg, view.width, gy);

                    if (style.label) {
                        fmt.str(std::string());
                        fmt << std::setprecision(std::max(3l, long(std::floor(std::log10(std::abs(y))) - scale.step_magnitude()) + 1))
                            << y;
                        nvgText(vg, label_x, gy + label_y, fmt.str().c_str(), nullptr);
                    }
                }

                nvgStroke(vg);
            }
        }
    }

    nvgRestore(vg);
}
