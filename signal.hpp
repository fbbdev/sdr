#pragma once

#include "stream/stream.hpp"

#include "opt/opt.hpp"

#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>

#include "kfr/math.hpp"

namespace sdr
{

typedef float __attribute__((aligned(512))) RealSample;
typedef kfr::complex<float> __attribute__((aligned(512))) Sample;

static constexpr Sample J{0, 1};

enum class FreqUnit {
    Hertz,
    Samples
};

class FreqUnitOption : public opt::EnumOption<FreqUnit> {
public:
    FreqUnitOption(opt::StringView k, FreqUnit v = FreqUnit())
        : opt::EnumOption<FreqUnit>(k, { { "hz",      FreqUnit::Hertz   },
                                         { "hertz",   FreqUnit::Hertz   },
                                         { "samples", FreqUnit::Samples } }, v)
        {}
};

enum class TimeUnit {
    Second,
    Samples
};

class TimeUnitOption : public opt::EnumOption<TimeUnit> {
public:
    TimeUnitOption(opt::StringView k, TimeUnit v = TimeUnit())
        : opt::EnumOption<TimeUnit>(k, { { "s",       TimeUnit::Second  },
                                         { "sec",     TimeUnit::Second  },
                                         { "seconds", TimeUnit::Second  },
                                         { "samples", TimeUnit::Samples } }, v)
        {}
};


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

template<typename T>
class PageAlignedAllocator {
public:
    using value_type = T;

    PageAlignedAllocator() noexcept = default;
    PageAlignedAllocator(PageAlignedAllocator const&) noexcept = default;
    PageAlignedAllocator(PageAlignedAllocator&&) noexcept = default;

    T* allocate(std::size_t n) const {
        T* ptr = reinterpret_cast<T*>(aligned_alloc(page_size, n*sizeof(T)));
        if (!ptr)
            throw std::bad_alloc();

        return ptr;
    }

    void deallocate(T* ptr, std::size_t) const noexcept {
        free(ptr);
    }

    bool operator==(PageAlignedAllocator const&) const noexcept {
        return true;
    }

    bool operator!=(PageAlignedAllocator const&) const noexcept {
        return false;
    }
};

} /* namespace sdr */
