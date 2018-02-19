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

#include "grid_view.hpp"

using namespace sdr::ui;

void GridView::draw(NVGcontext* vg, Rect r,
                    std::function<std::string(NVGcontext*, AppliedView,
                                              Vec2, bool)> const& content) const {
    auto view = compute(r);

    nvgSave(vg);
    nvgScissor(vg, r.x, r.y, r.w, r.h);

    // Background
    nvgSave(vg);
    nvgFillColor(vg, background_);
    nvgBeginPath(vg);
    nvgRect(vg, r.x, r.y, r.w, r.h);
    nvgFill(vg);
    nvgRestore(vg);

    grid_.draw(vg, view);

    // Content
    nvgSave(vg);
    auto label = content(vg, view, mouse, mouse_over);
    nvgRestore(vg);

    // Cursor
    if (show_cursor && (cursor_.mode() != Cursor::None || !label.empty()))
        cursor_.draw(vg, r, mouse, label);

    nvgRestore(vg);
}

void GridView::interact(Window* wnd, Rect r, bool zoom_around_cursor) {
    mouse = wnd->gui()->input.mouse.pos;
    mouse_over = wnd->mouse_over() &&
                 nk_input_is_mouse_hovering_rect(&wnd->gui()->input, r);
    show_cursor = mouse_over && wnd->focused() &&
                  wnd->cursor_mode() != Window::CursorMode::Grab;

    InteractiveView::interact(wnd, r, zoom_around_cursor);
}
