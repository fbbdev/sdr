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

#include "interactive_view.hpp"

using namespace sdr::ui;

void InteractiveView::interact(Window* wnd, Rect r, bool zoom_around_cursor) {
    auto ctx = wnd->gui();

    if (nk_input_is_mouse_hovering_rect(&ctx->input, r) && ctx->input.mouse.scroll_delta.y) {
        auto cursor = compute(r).local(ctx->input.mouse.pos);

        zoom(ctx->input.mouse.scroll_delta.y);
        if (zoom_around_cursor)
            move(nk_vec2_sub(compute(r).local(ctx->input.mouse.pos), cursor));
    }

    if (!pressed && nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_LEFT, r, true) &&
                    !nk_item_is_any_active(ctx)) {
        pressed = true;
        ctx->input.mouse.grab = true;
    } else if (pressed && nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)){
        move(compute(r).local_delta(ctx->input.mouse.delta));
    } else if (pressed) {
        pressed = false;
        ctx->input.mouse.ungrab = true;
    }

    // return nk_input_mouse_clicked(&ctx->input, NK_BUTTON_LEFT, r) && !nk_item_is_any_active(ctx);
}
