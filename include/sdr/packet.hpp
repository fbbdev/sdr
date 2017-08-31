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

#include <cstdint>
#include <complex>
#include <ostream>

namespace sdr
{

struct alignas(std::complex<double>) Packet {
    enum Content : std::uint16_t {
        Binary = 0,
        String,
        Time,
        Frequency,
        Wavelength,
        SampleCount,
        Signal,
        ComplexSignal,
        Spectrum,
        ComplexSpectrum,
    };

    std::uint16_t id;
    Content content;
    std::uint32_t size;
    std::uint64_t duration;

    template<typename T>
    bool compatible() const noexcept {
        return !(size % sizeof(T));
    }

    template<typename T>
    std::uint32_t count() const noexcept {
        return compatible<T>() ? (size / sizeof(T)) : 0;
    }
};

inline std::ostream& operator<<(std::ostream& stream, sdr::Packet::Content cnt) {
    using sdr::Packet;

    switch (cnt) {
        case Packet::Binary:
            return stream << "Binary";
        case Packet::String:
            return stream << "String";
        case Packet::Time:
            return stream << "Time";
        case Packet::Frequency:
            return stream << "Frequency";
        case Packet::Wavelength:
            return stream << "Wavelength";
        case Packet::SampleCount:
            return stream << "SampleCount";
        case Packet::Signal:
            return stream << "Signal";
        case Packet::ComplexSignal:
            return stream << "ComplexSignal";
        case Packet::Spectrum:
            return stream << "Spectrum";
        case Packet::ComplexSpectrum:
            return stream << "ComplexSpectrum";
    }

    return stream;
}

} /* namespace sdr */
