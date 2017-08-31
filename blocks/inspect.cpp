/**
 * sdr - software-defined radio building blocks for unix pipes
 * Copyright (C) 2017 Fabio Massaioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "options.hpp"
#include "stream.hpp"

#include <iostream>

using namespace sdr;

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<bool> pass("pass", false);
    Option<bool> pass_all("pass_all", false);

    if (!opt::parse({ id }, { pass, pass_all }, argv, argv + argc))
        return -1;

    if (!valid_stream_id(id.get())) {
        std::cerr << "error: gen: " << id.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

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
