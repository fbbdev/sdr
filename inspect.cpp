#include "options.hpp"
#include "stream/stream.hpp"

#include <iostream>

using namespace sdr;

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<bool> pass("pass", false);
    Option<bool> pass_all("pass_all", false);

    if (!opt::parse({ id }, { pass, pass_all }, argv, argv + argc))
        return -1;

    Source source;
    Sink sink;

    std::vector<std::uint8_t> buf;

    while (source.next()) {
        if (id.is_set() && source.packet().id != id) {
            if (pass || pass_all)
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

        if (pass_all)
            sink.send(pkt, buf);
    }

    return 0;
}
