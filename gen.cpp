#include "hilbert.hpp"
#include "options.hpp"
#include "signal.hpp"
#include "stream/stream.hpp"

#include "kfr/dsp/fir.hpp"
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
    Option<float> freq("freq", Placeholder("FREQ"), Required);
    FreqUnitOption unit(NoStream, "unit", FreqUnit::Hertz);
    EnumOption<Waveform> waveform({
        { "cosine", Cosine },
        { "sine", Sine },
        { "square", Square },
        { "triangle", Triangle },
        { "sawtooth", Sawtooth }
    }, "waveform", Cosine);
    Option<std::uintmax_t> sample_rate("sample_rate", Placeholder("HERTZ"), Required);
    Option<float> amplitude("amp", Placeholder("AMPLITUDE"), 1.0f);
    Option<float> phase("phi", Placeholder("PHASE"), 0.0f);
    EnumOption<Mode> mode({
        { "real", Real },
        { "complex", Complex }
    }, "mode", Complex);
    Option<std::uintmax_t> hilbert_taps("hilbert_taps", Placeholder("COUNT"), 127);
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);

    if (!opt::parse({ freq, unit, waveform },
                    { sample_rate, amplitude, phase, mode, hilbert_taps, id },
                    argv, argv + argc))
        return -1;

    if (!freq.is_set() || !sample_rate.is_set()) {
        std::cerr << "error: gen: freq and sample_rate options are required" << std::endl;
        opt::usage(argv[0],
                   { freq, unit, waveform },
                   { sample_rate, amplitude, phase, mode, hilbert_taps, id });
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
        std::uint16_t(id), (mode == Real) ? Packet::Signal : Packet::ComplexSignal,
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

        if (waveform == Cosine || waveform == Sine) {
            if (waveform == Sine)
                phi -= 0.25f;

            for (;;) {
                block = A*kfr::cexp(j2pi * (phi + cycles_per_sample*kfr::counter()));
                phi = kfr::fract(phi + phi_incr);
                sink.send(pkt, block_data);
            }
        } else {
            kfr::fir_state<RealSample> hilb(hilbert<RealSample>(hilbert_taps.get()));
            auto hilb_delay = (hilbert_taps - 1) / 2;

            switch (waveform.get()) {
                case Square:
                    block = kfr::truncate(kfr::fir(hilb, kfr::squarenorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);
                    for (;;) {
                        block = A*(kfr::squarenorm(phi + cycles_per_sample*kfr::counter()) +
                                   J*kfr::fir(hilb, kfr::squarenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                    cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, block_data);
                    }
                    break;
                case Triangle:
                    block = kfr::truncate(kfr::fir(hilb, kfr::trianglenorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);

                    for (;;) {
                        block = A*(kfr::trianglenorm(phi + cycles_per_sample*kfr::counter()) +
                                   J*kfr::fir(hilb, kfr::trianglenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                      cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, block_data);
                    }
                    break;
                case Sawtooth:
                    block = kfr::truncate(kfr::fir(hilb, kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);

                    for (;;) {
                        block = A*(kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter()) +
                                   J*kfr::fir(hilb, kfr::sawtoothnorm(phi + (cycles_per_sample*hilb_delay) +
                                                                      cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, block_data);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}
