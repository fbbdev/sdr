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
    bool any = true;
    std::uint16_t id = 0;

    if (argc > 1) {
        try {
            std::size_t count = 0;
            id = std::uint16_t(std::stoul(argv[1], &count));
            if (count < std::strlen(argv[1])) {
                std::cerr << "error: throttle: invalid stream id '" << argv[1] << "'" << std::endl;
                return -1;
            }

            any = false;
        } catch (...) {
            std::cerr << "error: throttle: invalid stream id '" << argv[1] << "'" << std::endl;
            return -1;
        }
    }

    auto source = stdin_source();
    auto sink = stdout_sink();

    std::vector<std::uint8_t> buf;

    while (source->next()) {
        if (!any && source->packet().id != id) {
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
