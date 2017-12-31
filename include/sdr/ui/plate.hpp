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

#include "string_view.hpp"
#include "view.hpp"

namespace sdr { namespace ui
{

struct PlateStyle {
    float padding = 5.0f;
    float margin = 5.0f;
    NVGcolor fg = nvgRGBf(1.0f, 1.0f, 1.0f);
    NVGcolor bg = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f);
    float radius = 3.0f;
};

class Plate {
public:
    explicit Plate(int a = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                   Vec2 p = { 0.0f, 0.0f })
        : align_(a), pos_(p)
        {}

    explicit Plate(PlateStyle s,
                   int a = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                   Vec2 p = { 0.0f, 0.0f })
        : style_(s), align_(a), pos_(p)
        {}

    explicit Plate(StringView l,
                   int a = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                   Vec2 p = { 0.0f, 0.0f })
        : label_(l.to_string()), align_(a), pos_(p)
        {}

    explicit Plate(StringView l, PlateStyle s,
                   int a = NVG_ALIGN_LEFT | NVG_ALIGN_TOP,
                   Vec2 p = { 0.0f, 0.0f })
        : label_(l.to_string()), style_(s), align_(a), pos_(p)
        {}

    StringView label() const {
        return label_;
    }

    PlateStyle const& style() const {
        return style_;
    }

    int align() const {
        return align_;
    }

    Vec2 pos() const {
        return pos_;
    }

    // Use default values
    void draw(NVGcontext* vg) const {
        draw(vg, label_, align_, pos_);
    }

    // Change position
    void draw(NVGcontext* vg, Vec2 p) const {
        draw(vg, label_, align_, p);
    }

    // Change alignment
    void draw(NVGcontext* vg, int a) const {
        draw(vg, label_, a, pos_);
    }

    // Change alignment and position
    void draw(NVGcontext* vg, int a, Vec2 p) const {
        draw(vg, label_, a, p);
    }

    // Change label
    void draw(NVGcontext* vg, StringView l) const {
        draw(vg, l, align_, pos_);
    }

    // Change label and position
    void draw(NVGcontext* vg, StringView l, Vec2 p) const {
        draw(vg, l, align_, p);
    }

    // Change label and alignment
    void draw(NVGcontext* vg, StringView l, int a) const {
        draw(vg, l, a, pos_);
    }

    // Draw plate
    void draw(NVGcontext* vg, StringView l, int a, Vec2 p) const;

private:
    std::string label_;
    PlateStyle style_;
    int align_;
    Vec2 pos_;
};

} /* namespace ui */ } /* namespace sdr */
