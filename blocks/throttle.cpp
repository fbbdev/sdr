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
