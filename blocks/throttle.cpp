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

#include <chrono>
#include <thread>

using namespace sdr;

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);

    if (!opt::parse({ id }, {}, argv, argv + argc))
        return -1;

    if (!valid_stream_id(id.get())) {
        std::cerr << "error: gen: " << id.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    Source source;
    Sink sink;

    auto next_packet = std::chrono::high_resolution_clock::now();
    while (source.next()) {
        source.pass(sink);

        if (!(id.is_set() && source.packet().id != id) && source.packet().duration) {
            next_packet += std::chrono::nanoseconds(source.packet().duration);
            std::this_thread::sleep_until(next_packet);
        }
    }

    return 0;
}
