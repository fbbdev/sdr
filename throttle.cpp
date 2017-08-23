#include "options/options.hpp"
#include "stream/stream.hpp"

#include <cstring>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace sdr;

int main(int argc, char* argv[]) {
    using opt::Option;
    using opt::Placeholder;

    Option<std::uintmax_t> id("stream", Placeholder("ID"), 0);

    if (!opt::parse({ id }, {}, argv, argv + argc))
        return -1;

    auto source = stdin_source();
    auto sink = stdout_sink();

    while (source->next()) {
        source->pass(sink);

        if (id.is_set() && source->packet().id != id)
            continue;

        if (source->packet().duration)
            std::this_thread::sleep_for(
                std::chrono::nanoseconds(source->packet().duration));
    }

    return 0;
}
