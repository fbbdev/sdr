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

#include "window.hpp"

namespace sdr { namespace ui { namespace detail {

constexpr int halign_mask = NVG_ALIGN_LEFT |
                            NVG_ALIGN_CENTER |
                            NVG_ALIGN_RIGHT;
constexpr int valign_mask = NVG_ALIGN_TOP |
                            NVG_ALIGN_MIDDLE |
                            NVG_ALIGN_BOTTOM |
                            NVG_ALIGN_BASELINE;

constexpr int halign(int align) {
    int a = align & halign_mask;
    return a ? a : NVG_ALIGN_LEFT;
}

constexpr int valign(int align) {
    int a = align & valign_mask;
    return a ? a : NVG_ALIGN_TOP;
}

constexpr int valign_inv(int align) {
    int a = valign(align);

    if (a & NVG_ALIGN_TOP)
        return NVG_ALIGN_BOTTOM;
    else if (a & NVG_ALIGN_BOTTOM)
        return NVG_ALIGN_TOP;

    return a;
}

} /* namespace detail */ } /* namespace ui */ } /* namespace sdr */
