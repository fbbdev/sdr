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

#include "view.hpp"
#include "window.hpp"

namespace sdr { namespace ui
{

class InteractiveView : public View {
public:
    using View::View;

    InteractiveView(View const& v) : View(v) {}

    void interact(Window* wnd, struct nk_rect r, bool zoom_around_cursor = false);

private:
    bool pressed = false;
};

} /* namespace ui */ } /* namespace sdr */
