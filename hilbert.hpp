#pragma once

#include "signal.hpp"

#include "kfr/dsp/window.hpp"

#include <cmath>

namespace sdr
{

// Hilbert transformer design function ripped from GNU Radio

template<typename T, std::size_t Tag>
void hilbert(kfr::univector<T, Tag>& taps) {
    auto size = taps.size() - !(taps.size() & 1);

    kfr::univector<T> wnd(size);
    wnd = kfr::window_blackman<T>(size);

    T gain = T();

    std::intmax_t half = size / 2;

    for (std::intmax_t i = 1; i <= half; ++i) {
        if (i & 1) {
            T x = T(1)/T(i);
            taps[half + i] = x * wnd[half + i];
            taps[half - i] = -x * wnd[half - i];
            gain = taps[half + i] - gain;
        } else {
            taps[half + i] = taps[half - i] = T();
        }
    }

    if (size < taps.size())
        taps.back() = T();

    gain = 2*kfr::abs(gain);
    for (auto& t: taps)
        t /= gain;
}

template<typename T, std::size_t Size>
inline kfr::univector<T, Size> hilbert() {
    static_assert(Size != kfr::tag_array_ref &&
                  Size != kfr::tag_dynamic_vector,
                  "Filter size is not a valid number");

    kfr::univector<T, Size> taps;
    hilbert(taps);
    return taps;
}

template<typename T>
inline kfr::univector<T> hilbert(std::size_t size) {
    kfr::univector<T> taps(size);
    hilbert(taps);
    return taps;
}

} /* namespace sdr */
