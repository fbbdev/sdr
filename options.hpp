#pragma once

#include "stream/packet.hpp"

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

enum NoStreamTag {
    NoStream
};

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

class FreqUnitOption : public EnumOption<FreqUnit> {
public:
    template<typename... Args>
    FreqUnitOption(Args&&... args)
        : EnumOption<FreqUnit>({ { "hz",      FreqUnit::Hertz   },
                                 { "hertz",   FreqUnit::Hertz   },
                                 { "m",       FreqUnit::Meter   },
                                 { "meter",   FreqUnit::Meter   },
                                 { "meters",  FreqUnit::Meter   },
                                 { "samples", FreqUnit::Samples },
                                 { "stream",  FreqUnit::Stream  } },
                               std::forward<Args>(args)...)
        {}

    template<typename... Args>
    FreqUnitOption(NoStreamTag, Args&&... args)
        : EnumOption<FreqUnit>({ { "hz",      FreqUnit::Hertz   },
                                 { "hertz",   FreqUnit::Hertz   },
                                 { "m",       FreqUnit::Meter   },
                                 { "meter",   FreqUnit::Meter   },
                                 { "meters",  FreqUnit::Meter   },
                                 { "samples", FreqUnit::Samples } },
                               std::forward<Args>(args)...)
        {}
};

enum class TimeUnit {
    Second,
    Samples,
    Stream,
};

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

class TimeUnitOption : public EnumOption<TimeUnit> {
public:
    template<typename... Args>
    TimeUnitOption(Args&&... args)
        : EnumOption<TimeUnit>({ { "s",       TimeUnit::Second  },
                                 { "sec",     TimeUnit::Second  },
                                 { "seconds", TimeUnit::Second  },
                                 { "samples", TimeUnit::Samples },
                                 { "stream",  TimeUnit::Stream  } },
                               std::forward<Args>(args)...)
        {}

    template<typename... Args>
    TimeUnitOption(NoStreamTag, Args&&... args)
        : EnumOption<TimeUnit>({ { "s",       TimeUnit::Second  },
                                 { "sec",     TimeUnit::Second  },
                                 { "seconds", TimeUnit::Second  },
                                 { "samples", TimeUnit::Samples } },
                               std::forward<Args>(args)...)
        {}
};

class PacketContentOption : public EnumOption<Packet::Content> {
public:
    template<typename... Args>
    PacketContentOption(Args&&... args)
        : EnumOption<Packet::Content>({
              { "binary",           Packet::Binary },
              { "string",           Packet::String },
              { "time",             Packet::Time },
              { "frequency",        Packet::Frequency },
              { "wavelength",       Packet::Wavelength },
              { "sample_count",     Packet::SampleCount },
              { "signal",           Packet::Signal },
              { "complex_signal",   Packet::ComplexSignal },
              { "spectrum",         Packet::Spectrum },
              { "complex_spectrum", Packet::ComplexSpectrum },
          }, std::forward<Args>(args)...)
        {}
};

} /* namespace sdr */
