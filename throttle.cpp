#include "options.hpp"
#include "stream/stream.hpp"

#include <chrono>
#include <thread>

using namespace sdr;

int main(int argc, char* argv[]) {
    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);

    if (!opt::parse({ id }, {}, argv, argv + argc))
        return -1;

    Source source;
    Sink sink;

    while (source.next()) {
        auto arrival = std::chrono::high_resolution_clock::now();

        source.pass(sink);

        if (!(id.is_set() && source.packet().id != id) && source.packet().duration)
            std::this_thread::sleep_until(
                arrival + std::chrono::nanoseconds(source.packet().duration));
    }

    return 0;
}
