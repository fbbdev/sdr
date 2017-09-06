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

#include <cmath>
#include <limits>

namespace sdr { namespace ui
{

using Vec2 = struct nk_vec2;

struct AppliedView {
    int width = 0, height = 0;
    Vec2 s = { 0.0f, 0.0f }, t = { 0.0f, 0.0f };

    Vec2 local(Vec2 p) const {
        return { p.x/s.x - t.x, p.y/s.y - t.y };
    }

    float local_x(float x) const {
        return x/s.x - t.x;
    }

    float local_y(float y) const {
        return y/s.y - t.y;
    }

    Vec2 local_delta(Vec2 p) const {
        return { p.x/s.x, p.y/s.y };
    }

    float local_delta_x(float x) const {
        return x/s.x;
    }

    float local_delta_y(float y) const {
        return y/s.y;
    }

    Vec2 global(Vec2 p) const {
        return { (p.x + t.x)*s.x, (p.y + t.y)*s.y };
    }

    float global_x(float x) const {
        return (x + t.x)*s.x;
    }

    float global_y(float y) const {
        return (y + t.y)*s.y;
    }

    Vec2 global_delta(Vec2 p) const {
        return { p.x*s.x, p.y*s.y };
    }

    float global_delta_x(float x) const {
        return x*s.x;
    }

    float global_delta_y(float y) const {
        return y*s.y;
    }

    void apply(NVGcontext* vg) const {
        nvgScale(vg, s.x, s.y);
        nvgTranslate(vg, t.x, t.y);
    }
};

class View {
public:
    enum IsometricMode {
        NonIsometric = 0,
        IsometricFitMin,
        IsometricFitMax,
    };

    View(Vec2 size = { 1.0f, 1.0f },
         Vec2 center = { 0.0f, 0.0f },
         Vec2 zoom_rate = { 1.1f, 1.1f })
        : iso(NonIsometric), size_(size), center_(center), rate(zoom_rate)
        {}

    View(IsometricMode mode,
         Vec2 size = { 1.0f, 1.0f },
         Vec2 center = { 0.0f, 0.0f },
         Vec2 zoom_rate = { 1.1f, 1.1f })
        : iso(mode), size_(size), center_(center), rate(zoom_rate)
        {}

    View(View const&) = default;
    View(View&&) = default;

    IsometricMode mode() const {
        return iso;
    }

    Vec2 size() const {
        return size_;
    }

    Vec2 center() const {
        return center_;
    }

    Vec2 zoom_rate() const {
        return rate;
    }

    void resize(Vec2 size) {
        size_ = size;
    }

    void place(Vec2 center) {
        center_ = center;
    }

    void move(Vec2 delta) {
        center_ = nk_vec2_sub(center_, delta);
    }

    void zoom(float exp) {
        zoom({exp, exp});
    }

    void zoom(Vec2 exp) {
        Vec2 fac{ std::pow(rate.x, -exp.x), std::pow(rate.y, -exp.y) };

        if ((exp.x < 0 && std::abs(size_.x) < std::numeric_limits<float>::max()/fac.x) ||
            (exp.x > 0 && std::abs(size_.x) > std::numeric_limits<float>::min()/fac.x))
            size_.x *= fac.x;

        if ((exp.y < 0 && std::abs(size_.y) < std::numeric_limits<float>::max()/fac.y) ||
            (exp.y > 0 && std::abs(size_.y) > std::numeric_limits<float>::min()/fac.y))
            size_.y *= fac.y;
    }

    AppliedView compute(int width, int height) const {
        auto mm = (width < height) ? std::pair<float, float>{ width, height }
                                   : std::pair<float, float>{ height, width };

        AppliedView v = {
            width, height,
            ((iso == NonIsometric) ? Vec2{float(width), float(height)} :
                 (iso == IsometricFitMin) ? Vec2{mm.first, mm.first}
                                          : Vec2{mm.second, mm.second}),
            {}
        };

        v.s.x /= size_.x;
        v.s.y /= size_.y;

        v.t = nk_vec2_sub(v.local_delta({ width/2.0f, height/2.0f }), center_);

        return v;
    }

    View& operator=(View const&) = default;
    View& operator=(View&&) = default;

private:
    IsometricMode iso;
    Vec2 size_, center_, rate;
};

} /* namespace ui */ } /* namespace sdr */
