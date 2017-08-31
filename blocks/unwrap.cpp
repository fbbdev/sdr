#include "options.hpp"
#include "stream.hpp"

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
    Sink sink(Raw);

    while (source.next()) {
        if (!id.is_set() || source.packet().id == id)
            source.pass(sink);
    }

    return 0;
}
