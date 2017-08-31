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

#include "packet.hpp"

#include "opt/opt.hpp"

#include <cmath>
#include <cstdint>
#include <limits>

namespace sdr
{

using opt::Option;
using opt::EnumOption;
using opt::Placeholder;
using opt::Required;

template<typename T, std::uintmax_t = 0>
struct AlternateOptionHelper {
    constexpr AlternateOptionHelper() = default;
    constexpr AlternateOptionHelper(T v_) : v(std::move(v_)) {}
    constexpr AlternateOptionHelper(AlternateOptionHelper const&) = default;

    constexpr operator T() const {
        return v;
    }

    constexpr bool operator==(AlternateOptionHelper const& other) const {
        return v == other.v;
    }

    constexpr bool operator!=(AlternateOptionHelper const& other) const {
        return v != other.v;
    }

    constexpr bool operator==(T const& other) const {
        return v == other;
    }

    constexpr bool operator!=(T const& other) const {
        return v != other;
    }

    AlternateOptionHelper& operator=(AlternateOptionHelper const&) = default;

    AlternateOptionHelper& operator=(T v_) {
        v = std::move(v_);
        return *this;
    }

    T v;
};

template<typename T, std::uintmax_t Id>
constexpr bool operator==(T const& lhs,
                          AlternateOptionHelper<T, Id> const& rhs) {
    return lhs == rhs.v;
}

template<typename T>
inline bool valid_stream_id(T const& id) {
    return id >= 0 && id <= std::numeric_limits<std::uint16_t>::max() &&
           id == std::floor(id);
}

template<typename T>
inline std::uint16_t convert_stream_id(T const& id) {
    return std::uint16_t(std::floor(id));
}

template<typename T>
inline T content_to_unit(Packet::Content cnt) {
    return T();
}

enum class FreqUnit {
    Hertz,
    Meter,
    Samples,
    Stream
};

using FreqUnitOption = Option<FreqUnit>;
using FreqUnitNoStream = AlternateOptionHelper<FreqUnit>;
using FreqUnitNoStreamOption = EnumOption<FreqUnitNoStream>;

template<>
inline FreqUnit content_to_unit<FreqUnit>(Packet::Content cnt) {
    switch (cnt) {
        case Packet::Frequency:
            return FreqUnit::Hertz;
        case Packet::Wavelength:
            return FreqUnit::Meter;
        case Packet::SampleCount:
            return FreqUnit::Samples;
        default:
            return FreqUnit::Stream;
    }
}

template<typename T = float>
inline T convert_freq(FreqUnit unit, T f, std::uintmax_t sample_rate) {
    switch (unit) {
        case FreqUnit::Meter:
            // c / lambda
            f = T(299792458) / f;
            [[fallthrough]];
        case FreqUnit::Hertz:
            f /= sample_rate;
            break;
        case FreqUnit::Samples:
            f = T(1) / f;
            break;
        default:
            break;
    }

    if (unit == FreqUnit::Stream || std::isnan(f) || std::isinf(f))
        return T();

    return f;
}

enum class TimeUnit {
    Second,
    Samples,
    Stream,
};

using TimeUnitOption = Option<TimeUnit>;
using TimeUnitNoStream = AlternateOptionHelper<TimeUnit>;
using TimeUnitNoStreamOption = EnumOption<TimeUnitNoStream>;

template<>
inline TimeUnit content_to_unit<TimeUnit>(Packet::Content cnt) {
    switch (cnt) {
        case Packet::Time:
            return TimeUnit::Second;
        case Packet::SampleCount:
            return TimeUnit::Samples;
        default:
            return TimeUnit::Stream;
    }
}

template<typename T = float>
inline T convert_time(TimeUnit unit, float t, std::uintmax_t sample_rate) {
    if (unit == TimeUnit::Second)
        t *= sample_rate;

    if (unit == TimeUnit::Stream || std::isnan(t) || std::isinf(t))
        return T();

    return t;
}

using PacketContentOption = Option<Packet::Content>;

} /* namespace sdr */

template<>
const sdr::FreqUnitOption::value_map sdr::FreqUnitOption::values = {
    { "hertz",   sdr::FreqUnit::Hertz   },
    { "hz",      sdr::FreqUnit::Hertz   },
    { "meters",  sdr::FreqUnit::Meter   },
    { "meter",   sdr::FreqUnit::Meter   },
    { "m",       sdr::FreqUnit::Meter   },
    { "samples", sdr::FreqUnit::Samples },
    { "stream",  sdr::FreqUnit::Stream  },
};

template<>
const sdr::FreqUnitNoStreamOption::value_map
sdr::FreqUnitNoStreamOption::values = {
    { "hertz",   sdr::FreqUnit::Hertz   },
    { "hz",      sdr::FreqUnit::Hertz   },
    { "meters",  sdr::FreqUnit::Meter   },
    { "meter",   sdr::FreqUnit::Meter   },
    { "m",       sdr::FreqUnit::Meter   },
    { "samples", sdr::FreqUnit::Samples },
};

template<>
const sdr::TimeUnitOption::value_map sdr::TimeUnitOption::values = {
    { "seconds", sdr::TimeUnit::Second  },
    { "sec",     sdr::TimeUnit::Second  },
    { "s",       sdr::TimeUnit::Second  },
    { "samples", sdr::TimeUnit::Samples },
    { "stream",  sdr::TimeUnit::Stream  },
};

template<>
const sdr::TimeUnitNoStreamOption::value_map
sdr::TimeUnitNoStreamOption::values = {
    { "seconds", sdr::TimeUnit::Second  },
    { "sec",     sdr::TimeUnit::Second  },
    { "s",       sdr::TimeUnit::Second  },
    { "samples", sdr::TimeUnit::Samples },
};

template<>
const sdr::PacketContentOption::value_map sdr::PacketContentOption::values = {
    { "binary",           sdr::Packet::Binary },
    { "string",           sdr::Packet::String },
    { "time",             sdr::Packet::Time },
    { "frequency",        sdr::Packet::Frequency },
    { "wavelength",       sdr::Packet::Wavelength },
    { "sample_count",     sdr::Packet::SampleCount },
    { "signal",           sdr::Packet::Signal },
    { "complex_signal",   sdr::Packet::ComplexSignal },
    { "spectrum",         sdr::Packet::Spectrum },
    { "complex_spectrum", sdr::Packet::ComplexSpectrum },
};
