#include "options/options.hpp"
#include "stream/stream.hpp"

#include <cstring>
#include <iostream>
#include <string>

using namespace sdr;

std::ostream& operator<<(std::ostream& stream, Packet::Content cnt) {
    switch (cnt) {
        case Packet::Binary:
            return stream << "Binary";
        case Packet::String:
            return stream << "String";
        case Packet::Frequency:
            return stream << "Frequency";
        case Packet::TimeSignal:
            return stream << "TimeSignal";
        case Packet::ComplexTimeSignal:
            return stream << "ComplexTimeSignal";
        case Packet::FreqSignal:
            return stream << "FreqSignal";
        case Packet::ComplexFreqSignal:
            return stream << "ComplexFreqSignal";
    }

    return stream;
}

int main(int argc, char* argv[]) {
    opt::Option<std::uintmax_t> id("stream", 0);

    if (!opt::parse({ id }, {}, argv + 1, argv + argc))
        return -1;

    auto source = stdin_source();
    auto sink = stdout_sink();

    std::vector<std::uint8_t> buf;

    while (source->next()) {
        if (id.is_set() && source->packet().id != id) {
            source->pass(sink);
            continue;
        }

        auto pkt = source->packet();
        std::cerr << "Packet{ "
                      << "id: "       << pkt.id       << ", "
                      << "content: "  << pkt.content  << ", "
                      << "size: "     << pkt.size     << ", "
                      << "duration: " << pkt.duration
                  << " }";

        buf.resize(pkt.size);
        auto r = source->recv(buf);
        std::cerr << " " << r << " bytes received" << std::endl;

        sink->send(pkt, buf);
    }

    return 0;
}
