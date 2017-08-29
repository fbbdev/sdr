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
    FreqUnitOption unit("unit", FreqUnit::Hertz);
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
        std::cerr << "error: gen: options freq and sample_rate are required" << std::endl;
        opt::usage(argv[0],
                   { freq, unit, waveform },
                   { sample_rate, amplitude, phase, mode, hilbert_taps, id });
        return -1;
    }

    if (unit == FreqUnit::Stream && (freq < 0 ||
                                     freq > std::numeric_limits<std::uint16_t>::max() ||
                                     freq != std::floor(freq))) {
        std::cerr << "error: gen: " << freq.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    float cycles_per_sample = 0.0f;
    if (unit != FreqUnit::Stream)
        cycles_per_sample = (unit == FreqUnit::Samples) ? 1.0f / freq.get()
                                                        : freq.get() / sample_rate.get();
    float A = amplitude;
    float phi = kfr::fract(phase / 360.0f);

    const std::size_t block_size =
        optimal_block_size((mode == Real) ? sizeof(RealSample) : sizeof(Sample), sample_rate);

    float phi_incr = kfr::fract(cycles_per_sample * block_size);

    Source source;
    Sink sink;

    const Packet pkt = {
        std::uint16_t(id), (mode == Real) ? Packet::Signal : Packet::ComplexSignal,
        std::uint32_t(block_size * ((mode == Real) ? sizeof(RealSample) : sizeof(Sample))),
        block_size*1000000000ull/sample_rate
    };

    std::vector<RealSample, SampleAllocator<RealSample>> real_data(block_size);
    std::vector<Sample, SampleAllocator<Sample>> complex_data(block_size);

    kfr::fir_state<RealSample> hilb(hilbert<RealSample>(hilbert_taps.get()));
    auto hilb_delay = (hilbert_taps - 1) / 2;

    const bool no_source = unit != FreqUnit::Stream;

    while (no_source || !source.end()) {
        if (!no_source) {
            float prev = cycles_per_sample;

            while (source.poll()) {
                if (!source.next())
                    break;

                if (source.packet().id == std::floor(freq) &&
                    (source.packet().content == Packet::Frequency ||
                     source.packet().content == Packet::SampleCount)) {
                    float data = 0.0f;
                    source.recv(&data, 1);
                    cycles_per_sample = (source.packet().content == Packet::SampleCount) ?
                        1.0f / data : data / sample_rate.get();
                }

                source.drop();
            }

            if (cycles_per_sample != prev) {
                phi_incr = kfr::fract(cycles_per_sample * block_size);

                if (mode == Complex) {
                    kfr::univector<Sample, 0> block(complex_data.data(), block_size);

                    switch (waveform.get()) {
                        case Square:
                            block = kfr::truncate(kfr::fir(hilb, kfr::squarenorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);
                            break;
                        case Triangle:
                            block = kfr::truncate(kfr::fir(hilb, kfr::trianglenorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);
                            break;
                        case Sawtooth:
                            block = kfr::truncate(kfr::fir(hilb, kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter())), hilb_delay);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (mode == Real) {
            kfr::univector<RealSample, 0> block(real_data.data(), block_size);

            switch (waveform.get()) {
                case Cosine:
                    do {
                        block = A*kfr::sinenorm(phi + 0.25f + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source);
                    break;
                case Sine:
                    do {
                        block = A*kfr::sinenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source);
                    break;
                case Square:
                    do {
                        block = A*kfr::squarenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source);
                    break;
                case Triangle:
                    do {
                        block = A*kfr::trianglenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source);
                    break;
                case Sawtooth:
                    do {
                        block = A*kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source);
                    break;
            }
        } else {
            kfr::univector<Sample, 0> block(complex_data.data(), block_size);

            const Sample j2pi = { 0, kfr::constants<RealSample>::pi_s(2) };

            if (waveform == Cosine || waveform == Sine) {
                if (waveform == Sine)
                    phi -= 0.25f;

                do {
                    block = A*kfr::cexp(j2pi * (phi + cycles_per_sample*kfr::counter()));
                    phi = kfr::fract(phi + phi_incr);
                    sink.send(pkt, complex_data);
                } while (no_source);
            } else {
                switch (waveform.get()) {
                    case Square:
                        do {
                            auto im = cycles_per_sample < 0 ? -J : J;
                            block = A*(kfr::squarenorm(phi + cycles_per_sample*kfr::counter()) +
                                       im*kfr::fir(hilb, kfr::squarenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                         cycles_per_sample*kfr::counter())));
                            phi = kfr::fract(phi + phi_incr);
                            sink.send(pkt, complex_data);
                        } while (no_source);
                        break;
                    case Triangle:
                        do {
                            auto im = cycles_per_sample < 0 ? -J : J;
                            block = A*(kfr::trianglenorm(phi + cycles_per_sample*kfr::counter()) +
                                       im*kfr::fir(hilb, kfr::trianglenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                           cycles_per_sample*kfr::counter())));
                            phi = kfr::fract(phi + phi_incr);
                            sink.send(pkt, complex_data);
                        } while (no_source);
                        break;
                    case Sawtooth:
                        do {
                            auto im = cycles_per_sample < 0 ? -J : J;
                            block = A*(kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter()) +
                                       im*kfr::fir(hilb, kfr::sawtoothnorm(phi + (cycles_per_sample*hilb_delay) +
                                                                           cycles_per_sample*kfr::counter())));
                            phi = kfr::fract(phi + phi_incr);
                            sink.send(pkt, complex_data);
                        } while (no_source);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return 0;
}
