#pragma once

#include "signal.hpp"

#include "kfr/dsp/fir.hpp"
#include "kfr/dsp/window.hpp"

#include <cmath>

namespace sdr
{

// Hilbert transformer design function ripped from GNU Radio

template<typename T, std::size_t Tag>
void hilbert(kfr::univector<T, Tag>& taps) {
    using base_type = decltype(kfr::real(T()));

    auto size = taps.size() - !(taps.size() & 1);

    kfr::univector<base_type> wnd(size);
    wnd = kfr::window_blackman<base_type>(size);

    base_type gain = base_type();

    std::intmax_t half = size / 2;

    for (std::intmax_t i = 1; i <= half; ++i) {
        if (i & 1) {
            base_type x = base_type(1)/base_type(i);
            taps[half + i] = x * wnd[half + i];
            taps[half - i] = -x * wnd[half - i];
            gain = kfr::real(taps[half + i]) - gain;
        } else {
            taps[half + i] = taps[half - i] = T();
        }
    }

    if (size < taps.size())
        taps.back() = T();

    gain = 2*kfr::abs(gain);
    for (auto& t: taps)
        t = t / gain;
}

template<typename T, std::size_t Size>
inline kfr::univector<T, Size> hilbert(kfr::csize_t<Size> = kfr::csize_t<Size>()) {
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
