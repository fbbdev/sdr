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

#include <cmath>
#include <iomanip>
#include <sstream>

namespace sdr { namespace ui
{

template<typename T>
inline std::string format(T const& v) {
    std::ostringstream fmt;
    fmt << v;
    return fmt.str();
}

inline std::string format(float v, long step_magnitude) {
    std::ostringstream fmt;
    fmt << std::defaultfloat
        << std::setprecision(std::max(3l, long(std::floor(std::log10(std::abs(v))) - step_magnitude) + 1))
        << v;
    return fmt.str();
}

inline std::string format(double v, long step_magnitude) {
    std::ostringstream fmt;
    fmt << std::defaultfloat
        << std::setprecision(std::max(3l, long(std::floor(std::log10(std::abs(v))) - step_magnitude) + 1))
        << v;
    return fmt.str();
}

} /* namespace ui */ } /* namespace sdr */
