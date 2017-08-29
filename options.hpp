#pragma once

#include "stream/packet.hpp"

#include "opt/opt.hpp"

namespace sdr
{

using opt::Option;
using opt::EnumOption;
using opt::Placeholder;
using opt::Required;

enum NoStreamTag {
    NoStream
};

enum class FreqUnit {
    Hertz,
    Samples,
    Stream
};

class FreqUnitOption : public EnumOption<FreqUnit> {
public:
    template<typename... Args>
    FreqUnitOption(Args&&... args)
        : EnumOption<FreqUnit>({ { "hz",      FreqUnit::Hertz   },
                                 { "hertz",   FreqUnit::Hertz   },
                                 { "samples", FreqUnit::Samples },
                                 { "stream",  FreqUnit::Stream  } },
                               std::forward<Args>(args)...)
        {}

    template<typename... Args>
    FreqUnitOption(NoStreamTag, Args&&... args)
        : EnumOption<FreqUnit>({ { "hz",      FreqUnit::Hertz   },
                                 { "hertz",   FreqUnit::Hertz   },
                                 { "samples", FreqUnit::Samples } },
                               std::forward<Args>(args)...)
        {}
};

enum class TimeUnit {
    Second,
    Samples,
    Stream,
};

class TimeUnitOption : public EnumOption<TimeUnit> {
public:
    template<typename... Args>
    TimeUnitOption(Args&&... args)
        : EnumOption<TimeUnit>({ { "s",       TimeUnit::Second  },
                                 { "sec",     TimeUnit::Second  },
                                 { "seconds", TimeUnit::Second  },
                                 { "samples", TimeUnit::Samples },
                                 { "stream",  TimeUnit::Stream  } },
                               std::forward<Args>(args)...)
        {}

    template<typename... Args>
    TimeUnitOption(NoStreamTag, Args&&... args)
        : EnumOption<TimeUnit>({ { "s",       TimeUnit::Second  },
                                 { "sec",     TimeUnit::Second  },
                                 { "seconds", TimeUnit::Second  },
                                 { "samples", TimeUnit::Samples } },
                               std::forward<Args>(args)...)
        {}
};

class PacketContentOption : public EnumOption<Packet::Content> {
public:
    template<typename... Args>
    PacketContentOption(Args&&... args)
        : EnumOption<Packet::Content>({
              { "binary", Packet::Binary },
              { "string", Packet::String },
              { "time", Packet::Time },
              { "frequency", Packet::Frequency },
              { "sample_count", Packet::SampleCount },
              { "signal", Packet::Signal },
              { "complex_signal", Packet::ComplexSignal },
              { "spectrum", Packet::Spectrum },
              { "complex_spectrum", Packet::ComplexSpectrum },
          }, std::forward<Args>(args)...)
        {}
};

} /* namespace sdr */
