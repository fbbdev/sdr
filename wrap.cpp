#include "options.hpp"
#include "signal.hpp"
#include "stream/stream.hpp"

using namespace sdr;

int main(int argc, char* argv[]) {
    PacketContentOption content("content_type", Packet::Binary);
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<std::uintmax_t> element_size("element_size", Placeholder("BYTES"), Required, 0);
    Option<std::uintmax_t> element_count("element_count", Placeholder("COUNT"), 0);
    Option<std::uintmax_t> duration("duration", Placeholder("NANOSECONDS"), 0);
    Option<std::uintmax_t> sample_rate("sample_rate", Placeholder("HERTZ"), 0);

    if (!opt::parse({ content, id },
                    { element_size, element_count, duration, sample_rate },
                    argv, argv + argc))
        return -1;

    if (!element_size.is_set()) {
        std::cerr << "error: wrap: element_size option is required" << std::endl;
        opt::usage(argv[0],
                   { content, id },
                   { element_size, element_count, duration, sample_rate });
        return -1;
    }

    if (element_size < 1) {
        std::cerr << "error: wrap: element_size must be greater than zero" << std::endl;
        return -1;
    }

    Packet pkt = {
        std::uint16_t(id), content.get(),
        0, std::uint64_t(duration.get())
    };

    if (element_count.is_set()) {
        if (element_count == 0) {
            std::cerr << "error: wrap: element_count must be greater than zero" << std::endl;
            return -1;
        }

        pkt.size = std::uint32_t(element_size * element_count);
    } else if (sample_rate.is_set() && duration.is_set()) {
        auto count = duration*sample_rate/1000000000ull;
        if (count < 1) {
            std::cerr << "error: wrap: packet duration is too small" << std::endl;
            return -1;
        }

        pkt.size = std::uint32_t(element_size * (duration*sample_rate/1000000000ull));
    } else {
        pkt.size = std::uint32_t(element_size * optimal_block_size(element_size, sample_rate));
    }

    if (sample_rate.is_set()) {
        pkt.duration = (pkt.size/element_size)*1000000000ull/sample_rate;

        if (duration.is_set() && element_count.is_set() && duration != pkt.duration) {
            std::cerr << "error: wrap: duration and element_count do not match" << std::endl;
            return -1;
        }
    }

    if (!content.is_set())
        std::cerr << "warning: wrap: content type not set, input will be "
                     "treated as binary data" << std::endl;

    Source source(Raw);
    Sink sink;

    while (source.next(pkt)) {
        while (!source.poll(-1));
        source.pass(sink);
    }

    return 0;
}
