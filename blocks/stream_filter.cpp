#include "options.hpp"
#include "stream.hpp"

using namespace sdr;

enum Mode {
    Pass,
    Drop,
};

template<>
const opt::Option<Mode>::value_map opt::Option<Mode>::values = {
    { "pass", Pass },
    { "drop", Drop },
};

int main(int argc, char* argv[]) {
    Option<Mode> mode("mode", Required);
    Option<std::set<std::uintmax_t>> ids("stream", Placeholder("ID,..."));
    Option<std::set<Packet::Content>> content("content");

    if (!opt::parse({ mode, ids, content }, {}, argv, argv + argc))
        return -1;

    if (!mode.is_set()) {
        std::cerr << "error: stream_filter: option 'mode' is required" << std::endl;
        opt::usage(argv[0], { mode, ids, content }, {});
        return -1;
    }

    for (auto id: ids.get()) {
        if (!valid_stream_id(id)) {
            std::cerr << "error: stream_filter: " << id << " is not a valid stream id" << std::endl;
            return -1;
        }
    }

    Source source;
    Sink sink;

    if (mode == Pass) {
        while (source.next()) {
            if (ids.is_set() && !ids.get().count(source.packet().id))
                continue;

            if (content.is_set() && !content.get().count(source.packet().content))
                continue;

            source.pass(sink);
        }
    } else {
        while (source.next()) {
            if (ids.is_set()) {
                if (ids.get().count(source.packet().id)) {
                    if (!content.is_set())
                        continue;
                    else if (content.get().count(source.packet().content))
                        continue;
                }
            } else if (content.is_set()) {
                if (content.get().count(source.packet().content))
                    continue;
            } else {
                continue;
            }

            source.pass(sink);
        }
    }
}
