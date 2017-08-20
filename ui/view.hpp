#pragma once

#include "ui.hpp"

#include <cmath>
#include <limits>

namespace sdr { namespace ui
{

using Vec2 = struct nk_vec2;

struct AppliedView {
    Vec2 s, t;

    Vec2 local(Vec2 p) const {
        return { p.x/s.x - t.x, p.y/s.y - t.y };
    }

    float local_x(float x) {
        return x/s.x - t.x;
    }

    float local_y(float y) {
        return y/s.y - t.y;
    }

    Vec2 local_delta(Vec2 p) const {
        return { p.x/s.x, p.y/s.y };
    }

    float local_delta_x(float x) {
        return x/s.x;
    }

    float local_delta_y(float y) {
        return y/s.y;
    }

    Vec2 global(Vec2 p) const {
        return { (p.x + t.x)*s.x, (p.y + t.y)*s.y };
    }

    float global_x(float x) {
        return (x + t.x)*s.x;
    }

    float global_y(float y) {
        return (y + t.y)*s.y;
    }

    Vec2 global_delta(Vec2 p) const {
        return { p.x*s.x, p.y*s.y };
    }

    float global_delta_x(float x) {
        return x*s.x;
    }

    float global_delta_y(float y) {
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
        center_ = nk_vec2_add(center_, delta);
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
        auto mm = (height > width) ? std::pair<float, float>{ height, width }
                                   : std::pair<float, float>{ width, height };

        AppliedView v = {
            ((iso == NonIsometric) ? Vec2{float(width), float(height)} :
                 (iso == IsometricFitMin) ? Vec2{mm.first, mm.first}
                                          : Vec2{mm.second, mm.second}),
            { size_.x/2.0f - center_.x, size_.y/2.0f - center_.y }
        };

        v.s.x /= size_.x;
        v.s.y /= size_.y;

        return v;
    }

    View& operator=(View const&) = default;
    View& operator=(View&&) = default;
private:
    IsometricMode iso;
    Vec2 size_, center_, rate;
};

} /* namespace ui */ } /* namespace sdr */
