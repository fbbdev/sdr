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

#include "align.hpp"
#include "format.hpp"
#include "grid.hpp"

using namespace sdr::ui;

void Grid::draw(NVGcontext* vg, AppliedView const& view) const {
    nvgSave(vg);

    for (auto const& lst: scales_) {
        auto const& style = lst.first;

        nvgStrokeColor(vg, style.color);
        nvgFillColor(vg, style.color);

        nvgStrokeWidth(vg, style.stroke_width);

        for (auto const& s: lst.second) {
            auto scale = s.compute(view);
            auto step_mag = scale.step_magnitude();

            if (scale.orientation() == Scale::Horizontal) {
                auto ha = detail::halign(style.label_align);

                nvgTextAlign(vg, ha | detail::valign(style.label_align));

                float label_x =
                    (ha & NVG_ALIGN_LEFT) ? style.label_distance :
                        (ha & NVG_ALIGN_RIGHT) ? -style.label_distance : 0.0f;
                float label_y = style.label_margin + (view.r.h - 2*style.label_margin)*style.label_position;

                nvgBeginPath(vg);

                for (auto x: scale) {
                    auto gx = view.global_x(x);
                    nvgMoveTo(vg, gx, view.r.y);
                    nvgLineTo(vg, gx, view.r.y + view.r.h);

                    if (style.label)
                        nvgText(vg, gx + label_x, view.r.y + label_y, format(x, step_mag).c_str(), nullptr);
                }

                nvgStroke(vg);
            } else {
                auto va = detail::valign_inv(style.label_align);

                nvgTextAlign(vg, detail::halign(style.label_align) | va);

                float label_x = style.label_margin + (view.r.w - 2*style.label_margin)*style.label_position;
                float label_y =
                    (va & NVG_ALIGN_TOP) ? style.label_distance :
                        (va & (NVG_ALIGN_BOTTOM | NVG_ALIGN_BASELINE)) ? -style.label_distance : 0.0f;

                nvgBeginPath(vg);

                for (auto y: scale) {
                    auto gy = view.global_y(y);
                    nvgMoveTo(vg, view.r.x, gy);
                    nvgLineTo(vg, view.r.x + view.r.w, gy);

                    if (style.label)
                        nvgText(vg, view.r.x + label_x, gy + label_y, format(y, step_mag).c_str(), nullptr);
                }

                nvgStroke(vg);
            }
        }
    }

    nvgRestore(vg);
}
