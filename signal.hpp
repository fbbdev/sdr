#pragma once

#include "stream/stream.hpp"

#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <new>

#include "kfr/math.hpp"

namespace sdr
{

typedef float __attribute__((aligned(512))) RealSample;
typedef kfr::complex<float> __attribute__((aligned(512))) Sample;

static constexpr Sample J{0, 1};

template<typename T>
class SampleAllocator {
public:
    using value_type = T;

    SampleAllocator() noexcept = default;
    SampleAllocator(SampleAllocator const&) noexcept = default;
    SampleAllocator(SampleAllocator&&) noexcept = default;

    T* allocate(std::size_t n) const {
        T* ptr = reinterpret_cast<T*>(aligned_alloc(512, n*sizeof(T)));
        if (!ptr)
            throw std::bad_alloc();

        return ptr;
    }

    void deallocate(T* ptr, std::size_t) const noexcept {
        free(ptr);
    }

    bool operator==(SampleAllocator const&) const noexcept {
        return true;
    }

    bool operator!=(SampleAllocator const&) const noexcept {
        return false;
    }
};

static const long page_size = sysconf(_SC_PAGESIZE);

inline std::size_t optimal_block_size(std::uintmax_t element_size, std::uintmax_t sample_rate = 0) {
    std::uintmax_t size =
        std::max(std::uintmax_t(1), std::uintmax_t(2*page_size - sizeof(Packet)) / element_size);

    if (sample_rate == 0)
        return std::size_t(size);

    const std::uintmax_t step =
        kfr::lcm(std::uintmax_t(sizeof(Packet)), element_size) / element_size;
    const std::uintmax_t limit =
        std::max(std::uintmax_t((page_size - sizeof(Packet)) / element_size), step);

    while (size > limit && (size * 1000000000ull) % sample_rate > 0)
        size -= step;

    return std::size_t(size);
}

} /* namespace sdr */
