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

#include "hilbert.hpp"
#include "options.hpp"
#include "signal.hpp"
#include "stream.hpp"

#include <algorithm>
#include <vector>

using namespace sdr;

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);
    Option<std::uintmax_t> taps("taps", Placeholder("TAPS"), 127);
    Option<bool> delay("delay", true);

    if (!opt::parse({ id, taps }, { delay }, argv, argv + argc))
        return -1;

    if (!valid_stream_id(id.get())) {
        std::cerr << "error: gen: " << id.get() << " is not a valid stream id" << std::endl;
        return -1;
    }

    Source source;
    Sink sink;

    kfr::fir_state<Sample> hilb(hilbert<Sample>(taps.get()));
    std::size_t hilb_delay = delay ? (std::size_t(taps.get()) - 1) / 2 : 0;

    std::vector<RealSample, SampleAllocator<RealSample>> real_data;
    std::vector<Sample, SampleAllocator<Sample>> input_data;

    std::vector<Sample, SampleAllocator<Sample>> output_data;

    while (source.next()) {
        auto pkt = source.packet();

        if (pkt.id != id || (pkt.content != Packet::Signal &&
                             pkt.content != Packet::ComplexSignal))
            source.pass(sink);

        if (pkt.content == Packet::Signal) {
            real_data.resize(pkt.count<RealSample>());
            real_data.resize(source.recv(real_data));
            input_data.resize(real_data.size());
            std::copy(real_data.begin(), real_data.end(), input_data.begin());
        } else {
            input_data.resize(pkt.count<Sample>());
            input_data.resize(source.recv(input_data));
        }

        output_data.resize(input_data.size());

        kfr::make_univector(output_data.data(), output_data.size()) =
            kfr::fir(hilb, kfr::make_univector(input_data.data(), input_data.size()));

        std::size_t skip = std::min(output_data.size(), hilb_delay);

        if (skip > 0) {
            hilb_delay -= skip;

            pkt.size -= skip * (pkt.content == Packet::Signal ? sizeof(RealSample)
                                                              : sizeof(Sample));
            pkt.duration -= (skip*pkt.duration)/input_data.size();

            if (pkt.size == 0)
                continue;
        }

        if (pkt.content == Packet::Signal) {
            real_data.resize(output_data.size() - skip);
            std::transform(output_data.begin() + skip, output_data.end(),
                           real_data.begin(), [](Sample s) { return s.real(); });
            sink.send(pkt, real_data);
        } else {
            sink.send(pkt, output_data.data() + skip, output_data.size() - skip);
        }
    }

    return 0;
}
