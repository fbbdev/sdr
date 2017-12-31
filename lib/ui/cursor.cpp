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
#include "cursor.hpp"

using namespace sdr::ui;

void Cursor::draw(NVGcontext* vg, Rect r, Vec2 pos, StringView label) const {
    nvgStrokeColor(vg, style_.color);
    nvgStrokeWidth(vg, style_.stroke_width);

    if (mode_ != None) {
        nvgBeginPath(vg);

        if (mode_ == Cross || mode_ == Horizontal) {
            nvgMoveTo(vg, r.y, pos.y);
            nvgLineTo(vg, r.y + r.w, pos.y);
        }

        if (mode_ == Cross || mode_ == Vertical) {
            nvgMoveTo(vg, pos.x, r.y);
            nvgLineTo(vg, pos.x, r.y + r.h);
        }

        nvgStroke(vg);
    }

    if (!label.empty()) {
        int halign = detail::halign(plate_.align());
        int valign = detail::valign(plate_.align());

        // Baseline alignment is not supported
        if (valign & NVG_ALIGN_BASELINE)
            valign = NVG_ALIGN_BOTTOM;

        if (halign & NVG_ALIGN_LEFT && ((r.x + r.w - pos.x) < style_.margin.x))
            halign = NVG_ALIGN_RIGHT; // Plate on the right, draw on the left when outside margin
        else if (halign & NVG_ALIGN_RIGHT && ((pos.x - r.x) < style_.margin.x))
            halign = NVG_ALIGN_LEFT; // Plate on the left, draw on the right when outside margin

        if (valign & NVG_ALIGN_TOP && ((r.y + r.h - pos.y) < style_.margin.y))
            valign = NVG_ALIGN_BOTTOM; // Plate below cursor, draw above when outside margin
        else if (valign & NVG_ALIGN_BOTTOM && ((pos.y - r.y) < style_.margin.y))
            valign = NVG_ALIGN_TOP; // Plate above cursor, draw below when outside margin

        plate_.draw(vg, label, halign | valign, pos);
    }
}
