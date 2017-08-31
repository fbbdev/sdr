#include "hilbert.hpp"
#include "options.hpp"
#include "signal.hpp"
#include "stream.hpp"

#include "kfr/dsp/oscillators.hpp"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace sdr;

enum Waveform {
    Cosine,
    Sine,
    Square,
    Triangle,
    Sawtooth
};

template<>
const opt::Option<Waveform>::value_map opt::Option<Waveform>::values = {
    { "cosine",   Cosine   },
    { "sine",     Sine     },
    { "square",   Square   },
    { "triangle", Triangle },
    { "sawtooth", Sawtooth },
};

enum Mode {
    Real,
    Complex
};

template<>
const opt::Option<Mode>::value_map opt::Option<Mode>::values = {
    { "real",    Real    },
    { "complex", Complex },
};

static std::atomic<float> freq_msg(0.0f);
static std::atomic<bool> source_end(false);
static std::atomic<bool> freq_msg_set(false);

void freq_input(std::uint16_t id, std::uintmax_t sample_rate) {
    Source source;

    while (source.next()) {
        auto unit = content_to_unit<FreqUnit>(source.packet().content);
        if (source.packet().id == id && unit != FreqUnit::Stream && source.packet().count<float>()) {
            float freq = 0.0f;
            source.recv(&freq, 1);
            freq_msg.store(convert_freq(unit, freq, sample_rate),
                           std::memory_order_relaxed);
            freq_msg_set.store(true, std::memory_order_release);
        }
    }

    source_end.store(true, std::memory_order_relaxed);
    freq_msg_set.store(true, std::memory_order_release);
}

int main(int argc, char* argv[]) {
    Option<float> freq("freq", Placeholder("FREQ"), Required);
    FreqUnitOption unit("unit", FreqUnit::Hertz);
    Option<Waveform> waveform("waveform", Cosine);
    Option<std::uintmax_t> sample_rate("sample_rate", Placeholder("HERTZ"), Required);
    Option<float> amplitude("amp", Placeholder("AMPLITUDE"), 1.0f);
    Option<float> phase("phi", Placeholder("PHASE"), 0.0f);
    Option<Mode> mode("mode", Complex);
    Option<std::uintmax_t> hilbert_taps("hilbert_taps", Placeholder("COUNT"), 127);
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);

    if (!opt::parse({ freq, unit, waveform },
                    { sample_rate, amplitude, phase, mode, hilbert_taps, id },
                    argv, argv + argc))
        return -1;

    if (!freq.is_set() || !sample_rate.is_set()) {
        std::cerr << "error: gen: options 'freq' and 'sample_rate' are required" << std::endl;
        opt::usage(argv[0],
                   { freq, unit, waveform },
                   { sample_rate, amplitude, phase, mode, hilbert_taps, id });
        return -1;
    }

    if (unit == FreqUnit::Stream && !valid_stream_id(freq.get())) {
        std::cerr << "error: gen: " << freq.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    if (!valid_stream_id(id.get())) {
        std::cerr << "error: gen: " << id.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    const std::size_t block_size =
        optimal_block_size((mode == Real) ? sizeof(RealSample) : sizeof(Sample), sample_rate);

    const float A = amplitude;

    float cycles_per_sample = convert_freq(unit, freq.get(), sample_rate);
    float phi = kfr::fract(phase / 360.0f);
    float phi_incr = kfr::fract(cycles_per_sample * block_size);

    Sink sink;

    const Packet pkt = {
        std::uint16_t(id), (mode == Real) ? Packet::Signal : Packet::ComplexSignal,
        std::uint32_t(block_size * ((mode == Real) ? sizeof(RealSample) : sizeof(Sample))),
        block_size*1000000000ull/sample_rate
    };

    std::vector<RealSample, SampleAllocator<RealSample>> real_data(mode == Real ? block_size : 0);
    std::vector<Sample, SampleAllocator<Sample>> complex_data(mode == Complex ? block_size : 0);

    const Sample j2pi = { 0, kfr::constants<RealSample>::pi_s(2) };
    kfr::fir_state<RealSample> hilb(hilbert<RealSample>(hilbert_taps.get()));
    const auto hilb_delay = (hilbert_taps - 1) / 2;

    const bool no_source = unit != FreqUnit::Stream;

    if (!no_source) {
        std::thread thr(freq_input, convert_stream_id(freq.get()), sample_rate);
        thr.detach();
    }

    if (mode == Complex && no_source) {
        kfr::univector<Sample> discard(hilb_delay);

        switch (waveform.get()) {
            case Square:
                discard = kfr::fir(hilb, kfr::squarenorm(phi + cycles_per_sample*kfr::counter()));
                break;
            case Triangle:
                discard = kfr::fir(hilb, kfr::trianglenorm(phi + cycles_per_sample*kfr::counter()));
                break;
            case Sawtooth:
                discard = kfr::fir(hilb, kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter()));
                break;
            default:
                break;
        }
    }

    for (;;) {
        if (!no_source && freq_msg_set.exchange(false, std::memory_order_acq_rel)) {
            if (source_end.load(std::memory_order_relaxed))
                break;

            float msg = freq_msg.load(std::memory_order_relaxed);
            if (msg != cycles_per_sample) {
                cycles_per_sample = msg;
                phi_incr = kfr::fract(cycles_per_sample * block_size);
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
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Sine:
                    do {
                        block = A*kfr::sinenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Square:
                    do {
                        block = A*kfr::squarenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Triangle:
                    do {
                        block = A*kfr::trianglenorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Sawtooth:
                    do {
                        block = A*kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter());
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, real_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
            }
        } else {
            kfr::univector<Sample, 0> block(complex_data.data(), block_size);

            switch (waveform.get()) {
                case Cosine:
                case Sine:
                    do {
                        block = A*kfr::cexp(j2pi * (phi + ((waveform == Sine) ? -0.25f : 0.0f) +
                                                    cycles_per_sample*kfr::counter()));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, complex_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Square:
                    do {
                        auto im = cycles_per_sample < 0 ? -J : J;
                        block = A*(kfr::squarenorm(phi + cycles_per_sample*kfr::counter()) +
                                   im*kfr::fir(hilb, kfr::squarenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                     cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, complex_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Triangle:
                    do {
                        auto im = cycles_per_sample < 0 ? -J : J;
                        block = A*(kfr::trianglenorm(phi + cycles_per_sample*kfr::counter()) +
                                   im*kfr::fir(hilb, kfr::trianglenorm(phi + (cycles_per_sample*hilb_delay) +
                                                                       cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, complex_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                case Sawtooth:
                    do {
                        auto im = cycles_per_sample < 0 ? -J : J;
                        block = A*(kfr::sawtoothnorm(phi + cycles_per_sample*kfr::counter()) +
                                   im*kfr::fir(hilb, kfr::sawtoothnorm(phi + (cycles_per_sample*hilb_delay) +
                                                                       cycles_per_sample*kfr::counter())));
                        phi = kfr::fract(phi + phi_incr);
                        sink.send(pkt, complex_data);
                    } while (no_source || !freq_msg_set.load(std::memory_order_acquire));
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}
