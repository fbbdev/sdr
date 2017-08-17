#include "stream/stream.hpp"

#include <cstring>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace sdr;

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

    while (source->next()) {
        source->pass(sink);

        if (!any && source->packet().id != id)
            continue;

        if (source->packet().duration)
            std::this_thread::sleep_for(
                std::chrono::nanoseconds(source->packet().duration));
    }
    
    return 0;
}
