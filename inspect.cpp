#include "stream/stream.hpp"

#include "opt/opt.hpp"

#include <cstring>
#include <iostream>
#include <string>

using namespace sdr;

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
