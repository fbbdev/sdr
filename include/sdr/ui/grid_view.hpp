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

#include "cursor.hpp"
#include "grid.hpp"
#include "plate.hpp"
#include "interactive_view.hpp"
#include "window.hpp"

#include <functional>

namespace sdr { namespace ui
{

class GridView : public InteractiveView {
public:
    GridView(NVGcolor background, Cursor cursor, Grid grid, View const& view)
        : InteractiveView(view),
          background_(background),
          cursor_(cursor),
          grid_(std::move(grid))
        {}

    void draw(NVGcontext* vg, Rect r, std::function<std::string(NVGcontext*, AppliedView, Vec2, bool)> const& content) const;

    void draw(NVGcontext* vg, Rect r, std::function<std::string(NVGcontext*, AppliedView)> const& content) const {
        draw(vg, r, [&content](NVGcontext* vg, AppliedView view, Vec2, bool) -> std::string {
            return content(vg, view);
        });
    }

    void interact(Window* wnd, Rect r, bool zoom_around_cursor = false);

private:
    NVGcolor background_;
    Cursor cursor_;
    Grid grid_;

    Vec2 mouse = { 0.0f, 0.0f };
    bool mouse_over = false;
    bool show_cursor = false;
};

} /* namespace ui */ } /* namespace sdr */
