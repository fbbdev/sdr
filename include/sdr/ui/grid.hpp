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

#include "ui.hpp"
#include "view.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <limits>
#include <vector>

namespace sdr { namespace ui
{

class Scale {
public:
    enum Orientation {
        Horizontal,
        Vertical
    };

    // Container API
    using value_type = float;
    using reference_type = value_type;
    using const_reference = value_type;
    using iterator = Scale;
    using const_iterator = Scale;
    using reverse_iterator = std::reverse_iterator<Scale>;
    using const_reverse_iterator = std::reverse_iterator<Scale>;
    using difference_type = std::intptr_t;
    using size_type = std::size_t;

    // Iterator API
    using iterator_category = std::random_access_iterator_tag;
    using reference = const_reference;
    using pointer = value_type const*;

    // Scale API
    Scale() = default;

    Scale(Orientation o, float off, float s)
        : orientation_(o), offset_(off), step_(s)
        {}

    Scale(Orientation o, float off, std::pair<float, float> b, float s)
        : orientation_(o), offset_(off), bounds_(b), step_(s)
        {}

    Scale(Orientation o, float off, int w, float m)
        : orientation_(o), offset_(off), width(w), multiplier(m)
        {}

    Scale(Orientation o, float off, std::pair<float, float> b, int w, float m)
        : orientation_(o), offset_(off), bounds_(b), width(w), multiplier(m)
        {}

    Scale(Orientation o, float off, float s, AppliedView const& view)
        : orientation_(o), offset_(off), step_(s)
    {
        *this = compute(view);
    }

    Scale(Orientation o, float off, std::pair<float, float> b,
          float s, AppliedView const& view)
        : orientation_(o), offset_(off), bounds_(b), step_(s)
    {
        *this = compute(view);
    }

    Scale(Orientation o, float off, int w, float m, AppliedView const& view)
        : orientation_(o), offset_(off), width(w), multiplier(m)
    {
        *this = compute(view);
    }

    Scale(Orientation o, float off, std::pair<float, float> b,
          int w, float m, AppliedView const& view)
        : orientation_(o), offset_(off), bounds_(b), width(w), multiplier(m)
    {
        *this = compute(view);
    }

    Scale(Scale const&) = default;
    Scale(Scale&&) = default;

    Orientation orientation() const {
        return orientation_;
    }

    float offset() const {
        return offset_;
    }

    std::pair<float, float> bounds() const {
        return bounds_;
    }

    float step() const {
        return step_;
    }

    float step(AppliedView const& view) const {
        return width != 0 ? compute_step(view) : step_;
    }

    long step_magnitude() const {
        return long(std::floor(std::log10(step_)));
    }

    long step_magnitude(AppliedView const& view) const {
        return long(std::floor(std::log10(step(view))));
    }

    Scale compute(AppliedView const& view) const {
        Scale s(orientation_, offset_, width, multiplier);
        s.step_ = step(view);

        if (s.step_ == 0.0f || std::isnan(s.step_) || std::isinf(s.step_)) {
            s.step_ = 0.0f;
            return s;
        }

        std::pair<float, float> sides = (orientation_ == Horizontal) ?
            std::minmax(view.local_x(0), view.local_x(view.r.w)) :
            std::minmax(view.local_y(0), view.local_y(view.r.h));

        float start_pos = std::ceil(std::max(sides.first/s.step_ - offset_ - 1.0f, bounds_.first));
        s.start = s.value = s.step_ * (start_pos + offset_);
        float end_pos = 1.0f + std::floor(std::min(sides.second/s.step_ - offset_ + 1.0f, bounds_.second));
        s.size_ = std::size_t(std::max(0.0f, end_pos - start_pos));

        return s;
    }

    // Container API
    size_type size() const {
        return size_;
    }

    size_type max_size() const {
        return std::numeric_limits<size_type>::max();
    }

    bool empty() const {
        return size() == 0;
    }

    const_reference front() const {
        return *begin();
    }

    const_reference back() const {
        return *end();
    }

    const_reference at(difference_type i) const {
        return *(begin() + i);
    }

    const_iterator begin() const {
        Scale s = *this;
        s.pos = 0;
        s.value = start;
        return s;
    }

    const_iterator end() const {
        return begin() + size();
    }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const { return rbegin(); }
    const_reverse_iterator crend() const { return rend(); }

    void swap(Scale& other) {
        std::swap(orientation_, other.orientation_);
        std::swap(offset_, other.offset_);
        std::swap(step_, other.step_);
        std::swap(width, other.width);
        std::swap(multiplier, other.multiplier);
        std::swap(start, other.start);
        std::swap(size_, other.size_);
        std::swap(pos, other.pos);
        std::swap(value, other.value);
    }

    // Iterator API
    reference operator*() const {
        return value;
    }

    pointer operator->() const {
        return &value;
    }

    reference operator[](difference_type i) const {
        return *(*this + i);
    }

    Scale& operator++() {
        value += step_;
        ++pos;
        return *this;
    }

    Scale operator++(int) {
        Scale s = *this;
        ++(*this);
        return s;
    }

    Scale& operator--() {
        value -= step_;
        --pos;
        return *this;
    }

    Scale operator--(int) {
        Scale s = *this;
        --(*this);
        return s;
    }

    Scale& operator+=(difference_type i) {
        pos += i;
        value = start + pos*step_;
        return *this;
    }

    Scale& operator-=(difference_type i) {
        pos -= i;
        value = start + pos*step_;
        return *this;
    }

    Scale operator+(difference_type i) const {
        Scale s = *this;
        s += i;
        return s;
    }

    Scale operator-(difference_type i) const {
        Scale s = *this;
        s -= i;
        return s;
    }

    difference_type operator-(Scale const& other) const {
        return pos - other.pos;
    }

    bool operator==(Scale const& other) const {
        return pos == other.pos;
    }

    bool operator!=(Scale const& other) const {
        return pos != other.pos;
    }

    bool operator<(Scale const& other) const {
        return pos < other.pos;
    }

    bool operator<=(Scale const& other) const {
        return pos <= other.pos;
    }

    bool operator>(Scale const& other) const {
        return pos > other.pos;
    }

    bool operator>=(Scale const& other) const {
        return pos >= other.pos;
    }

    Scale& operator=(Scale const&) = default;
    Scale& operator=(Scale&&) = default;

private:
    float compute_step(AppliedView const& view) const {
        return multiplier * std::pow(2.0f, std::round(std::log2(std::abs(
            (orientation_ == Horizontal) ? view.local_delta_x(width)
                                         : view.local_delta_y(width)))));
    }

    Orientation orientation_;
    float offset_;
    std::pair<float, float> bounds_ = { -INFINITY, INFINITY };
    float step_ = 0.0f;
    int width = 0;
    float multiplier;

    value_type start = 0.0f;
    size_type size_ = 0;
    difference_type pos = 0;
    value_type value = 0.0f;
};

Scale operator+(Scale::difference_type i, Scale const& s) {
    return s + i;
}

struct GridStyle {
    NVGcolor color;
    float stroke_width;

    bool label = true;
    float label_distance = 5;
    float label_margin = 5;
    float label_position = 0;
    int label_align = NVG_ALIGN_LEFT | NVG_ALIGN_TOP;
};

class Grid {
public:
    using styled_scale_list = std::vector<std::pair<GridStyle, std::vector<Scale>>>;

    Grid() = default;

    Grid(styled_scale_list const& lst) : scales_(lst) {}
    Grid(styled_scale_list&& lst) : scales_(std::move(lst)) {}

    Grid(Grid const&) = default;
    Grid(Grid&&) = default;

    styled_scale_list const& scales() const {
        return scales_;
    }

    void draw(NVGcontext* vg, AppliedView const& view) const;

    Grid& operator=(Grid const&) = default;
    Grid& operator=(Grid&&) = default;
private:
    styled_scale_list scales_;
};

} /* namespace ui */ } /* namespace sdr */
