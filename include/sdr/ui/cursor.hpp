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

#pragma once

#include "plate.hpp"
#include "string_view.hpp"
#include "view.hpp"
#include "window.hpp"

namespace sdr { namespace ui
{

struct CursorStyle {
    NVGcolor color = nvgRGBf(0.6f, 0.6f, 0.6f);
    float stroke_width = 1.0f;
    Vec2 margin = { 100, 50 };
};

class Cursor {
public:
    enum Mode {
        None,
        Cross,
        Horizontal,
        Vertical,
    };

    explicit Cursor(Mode mode, CursorStyle style, Plate plate = Plate())
        : mode_(mode), style_(style), plate_(plate)
        {}

    explicit Cursor(CursorStyle style, Plate plate = Plate())
        : mode_(None), style_(style), plate_(plate)
        {}

    explicit Cursor(Mode mode, Plate plate)
        : mode_(mode), style_(), plate_(plate)
        {}

    explicit Cursor(Plate plate)
        : mode_(None), style_(), plate_(plate)
        {}

    Cursor(Mode mode = None)
        : mode_(mode), style_(), plate_()
        {}

    Mode mode() const {
        return mode_;
    }

    CursorStyle const& style() const {
        return style_;
    }

    Plate const& plate() const {
        return plate_;
    }

    void draw(NVGcontext* vg, Rect r, Vec2 pos, StringView label = StringView()) const;

private:
    Mode mode_;
    CursorStyle style_;
    Plate plate_;
};

} /* namespace ui */ } /* namespace sdr */
