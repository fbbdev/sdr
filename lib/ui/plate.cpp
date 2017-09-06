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
#include "plate.hpp"

using namespace sdr::ui;

void Plate::draw(NVGcontext* vg, StringView l, int a, Vec2 p) const {
    nvgSave(vg);

    auto halign = detail::halign(a),
         valign = detail::valign(a);

    // Baseline alignment is not supported
    if (valign & NVG_ALIGN_BASELINE)
        valign = NVG_ALIGN_BOTTOM;

    float label_x =
        (halign & NVG_ALIGN_LEFT) ? p.x + style.margin + style.padding :
            (halign & NVG_ALIGN_RIGHT) ? p.x - style.margin - style.padding : p.x;
    float label_y =
        (valign & NVG_ALIGN_TOP) ? p.y + style.margin + style.padding :
            (valign & NVG_ALIGN_BOTTOM) ? p.y - style.margin - style.padding : p.y;

    float bounds[4];
    nvgTextBounds(vg, label_x, label_y, l.begin(), l.end(), bounds);
    float label_width = std::abs(bounds[2] - bounds[0]);
    float label_height = std::abs(bounds[3] - bounds[1]);

    float plate_width = label_width + 2*style.padding;
    float plate_height = label_height + 2*style.padding;

    float plate_x =
        (halign & NVG_ALIGN_LEFT) ? p.x + style.margin :
            (halign & NVG_ALIGN_RIGHT) ? p.x - style.margin - plate_width : p.x - (plate_width/2);
    float plate_y =
        (valign & NVG_ALIGN_TOP) ? p.y + style.margin :
            (valign & NVG_ALIGN_BOTTOM) ? p.y - style.margin - plate_height : p.y - (plate_height/2);

    nvgFillColor(vg, style.bg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, plate_x, plate_y, plate_width, plate_height, style.radius);
    nvgFill(vg);

    nvgFillColor(vg, style.fg);
    nvgTextAlign(vg, halign | valign);
    nvgText(vg, label_x, label_y, l.begin(), l.end());

    nvgRestore(vg);
}
