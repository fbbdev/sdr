#include "stream/stream.hpp"

#include "opt/opt.hpp"

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
        if (!id.is_set() || source->packet().id == id)
            source->pass(sink, true);
    }

    return 0;
}
