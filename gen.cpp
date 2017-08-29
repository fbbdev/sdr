#include "signal.hpp"
#include "stream/stream.hpp"

#include "opt/opt.hpp"

#include "kfr/dsp/oscillators.hpp"

#include <iostream>
#include <vector>

using namespace sdr;

enum Waveform {
    Cosine,
    Sine,
    Square,
    Triangle,
    Sawtooth
};

enum Mode {
    Real,
    Complex
};

int main(int argc, char* argv[]) {
    using opt::Option;
    using opt::EnumOption;
    using opt::Placeholder;
    using opt::Required;

    Option<float> freq("freq", Placeholder("FREQ"), Required);
    FreqUnitOption unit("unit", FreqUnit::Hertz);
    EnumOption<Waveform> waveform("waveform", {
        { "cosine", Cosine },
        { "sine", Sine },
        { "square", Square },
        { "triangle", Triangle },
        { "sawtooth", Sawtooth }
    }, Cosine);
    Option<std::uintmax_t> sample_rate("sample_rate", Placeholder("RATE"), Required);
    Option<float> amplitude("amp", Placeholder("AMPLITUDE"), 1.0f);
    Option<float> phase("phi", Placeholder("PHASE"), 0.0f);
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    EnumOption<Mode> mode("mode", {
        { "real", Real },
        { "complex", Complex }
    }, Complex);

    if (!opt::parse({ freq, unit, waveform },
                    { sample_rate, amplitude, phase, mode, id },
                    argv, argv + argc))
        return -1;

    if (!freq.is_set() || !sample_rate.is_set()) {
        opt::usage(argv[0],
                   { freq, unit, waveform },
                   { sample_rate, amplitude, phase, mode, id });
        return -1;
    }

    const float cycles_per_sample =
        (unit == FreqUnit::Samples) ? 1.0f / freq.get()
                                    : freq.get() / sample_rate.get();
    const float A = amplitude;
    float phi = kfr::fract(phase / 360.0f);

    const std::size_t block_size =
        optimal_block_size((mode == Real) ? sizeof(RealSample) : sizeof(Sample), sample_rate);

    const float phi_incr = kfr::fract(cycles_per_sample * block_size);

    Sink sink;
    const Packet pkt = {
        std::uint16_t(id), (mode == Real) ? Packet::TimeSignal : Packet::ComplexTimeSignal,
        std::uint32_t(block_size * ((mode == Real) ? sizeof(RealSample) : sizeof(Sample))),
        block_size*1000000000ull/sample_rate
    };

    if (mode == Real) {
        std::vector<RealSample, SampleAllocator<RealSample>> block_data(block_size);
        kfr::univector<RealSample, 0> block(block_data.data(), block_size);

        switch (waveform.get()) {
            case Cosine:
                for (;;) {
                    block = A*kfr::sinenorm(phi + 0.25f + cycles_per_sample*kfr::counter());
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Sine:
                for (;;) {
                    block = A*kfr::sinenorm(phi + cycles_per_sample*kfr::counter());
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Square:
                for (;;) {
                    block = A*kfr::squarenorm(phi + cycles_per_sample*kfr::counter());
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Triangle:
                for (;;) {
                    block = A*kfr::trianglenorm(phi + cycles_per_sample*kfr::counter());
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Sawtooth:
                for (;;) {
                    block = A*kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter());
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
        }
    } else {
        std::vector<Sample, SampleAllocator<Sample>> block_data(block_size);
        kfr::univector<Sample, 0> block(block_data.data(), block_size);

        const Sample j2pi = { 0, kfr::constants<RealSample>::pi_s(2) };

        switch (waveform.get()) {
            case Cosine:
                for (;;) {
                    block = A*kfr::cexp(j2pi * (phi + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Sine:
                for (;;) {
                    block = A*kfr::cexp(j2pi * (phi - 0.25f + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Square:
                for (;;) {
                    block = A*(kfr::squarenorm(phi + cycles_per_sample*kfr::counter()) +
                               J*kfr::squarenorm(phi - 0.25f + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Triangle:
                for (;;) {
                    block = A*(kfr::trianglenorm(phi + cycles_per_sample*kfr::counter()) +
                               J*kfr::trianglenorm(phi - 0.25f + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
            case Sawtooth:
                for (;;) {
                    block = A*(kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter()) +
                               J*kfr::sawtoothnorm(phi - 0.25f + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, block_data);
                }
                break;
        }
    }

    return 0;
}
