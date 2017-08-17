#include "stream/stream.hpp"

#include <cmath>
#include <algorithm>
#include <complex>

using namespace sdr;

int main() {
    std::vector<std::complex<float>> signal(960);

    const float pi = std::acos(-1.0f);
    const float rad_sample = 2.0f * pi / 960;

    auto sink = stdout_sink();

    const float phi_incr = 0.83f * rad_sample;
    float phi = 0;

    for (;;) {
        for (int i = 0; i < 960; ++i) {
            signal[i] =
                std::exp(std::complex<float>(0, rad_sample*i)) +
                0.4f*std::exp(std::complex<float>(0, 8*rad_sample*i + phi));
        }

        phi += phi_incr;
        if (phi > 2.0f * pi)
            phi -= 2.0f * pi;

        sink->send(0, Packet::ComplexTimeSignal, 20000000 /*ns*/, signal);
    }
}
