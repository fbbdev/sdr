#pragma once

#include <atomic>

namespace sdr
{

class Spinlock
{
public:
    void lock() {
        while (!try_lock())
            asm volatile ("pause":::"memory");
    }

    bool try_lock() {
        return !locked.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        locked.clear(std::memory_order_release);
    }

private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
};

} /* namespace sdr */
