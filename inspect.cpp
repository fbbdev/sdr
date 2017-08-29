#include "stream/stream.hpp"

#include "opt/opt.hpp"

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
        case Packet::Time:
            return stream << "Time";
        case Packet::Frequency:
            return stream << "Frequency";
        case Packet::Signal:
            return stream << "Signal";
        case Packet::ComplexSignal:
            return stream << "ComplexSignal";
        case Packet::Spectrum:
            return stream << "Spectrum";
        case Packet::ComplexSpectrum:
            return stream << "ComplexSpectrum";
    }

    return stream;
}

int main(int argc, char* argv[]) {
    using opt::Option;
    using opt::Placeholder;

    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<bool> tap("tap", false);

    if (!opt::parse({ id }, { tap }, argv, argv + argc))
        return -1;

    Source source;
    Sink sink;

    std::vector<std::uint8_t> buf;

    while (source.next()) {
        if (id.is_set() && source.packet().id != id) {
            if (tap)
                source.pass(sink);
            continue;
        }

        auto pkt = source.packet();
        std::cerr << "Packet{ "
                      << "id: "       << pkt.id       << ", "
                      << "content: "  << pkt.content  << ", "
                      << "size: "     << pkt.size     << ", "
                      << "duration: " << pkt.duration
                  << " }";

        buf.resize(pkt.size);
        auto r = source.recv(buf);
        std::cerr << " " << r << " bytes received" << std::endl;

        if (tap)
            sink.send(pkt, buf);
    }

    return 0;
}
