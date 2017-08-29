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

} /* namespace sdr */

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
