#pragma once

#include <cstdint>
#include <chrono>
#include <complex>
#include <memory>
#include <vector>

namespace sdr
{

struct alignas(std::complex<double>) Packet {
    enum Content : std::uint16_t {
        Binary = 0,
        String,
        Frequency,
        TimeSignal,
        ComplexTimeSignal,
        FreqSignal,
        ComplexFreqSignal,
    };

    std::uint16_t id;
    Content content;
    std::uint32_t size;
    std::uint64_t duration;

    template<typename T>
    bool compatible() const noexcept {
        return !(size % sizeof(T));
    }

    template<typename T>
    std::uint32_t count() const noexcept {
        return compatible<T>() ? (size / sizeof(T)) : 0;
    }
};

class Source;
class FileSource;
class Sink;
class FileSink;

class Source {
public:
    explicit Source(int fd_ = 0) : fd(fd_) {}

    virtual ~Source() {}

    bool next();

    bool end() const noexcept {
        return eof;
    }

    Packet const& packet() const noexcept {
        return pkt;
    }

    template<typename T>
    std::vector<T> recv() {
        if (!pkt.compatible<T>())
            return std::vector<T>();

        std::vector<T> data(pkt.count<T>());

        auto read = recv(data);
        data.resize(read);

        return data;
    }

    template<typename T>
    std::uint32_t recv(std::vector<T>& data) {
        return recv(data.data(), data.size());
    }

    template<typename T>
    std::uint32_t recv(T* data, std::uint32_t count = 0) {
        if (!pkt.compatible<T>())
            return 0;

        return recv(reinterpret_cast<std::uint8_t*>(data), count*sizeof(T))/sizeof(T);
    }

    std::uint32_t recv(std::uint8_t* data, std::uint32_t size = 0);

    virtual void drop();

    virtual void pass(Sink& sink);
    virtual void pass(FileSink& sink);

    virtual void copy(Sink& sink);
    virtual void copy(FileSink& sink);

    void pass(Sink* sink);
    void copy(Sink* sink);

    void pass(std::unique_ptr<Sink> const& sink) {
        pass(sink.get());
    }

    void copy(std::unique_ptr<Sink> const& sink) {
        copy(sink.get());
    }

protected:
    int fd;

    Packet pkt{};
    std::uint32_t read = 0;
    bool eof = false;

    std::vector<std::uint8_t> buffer;
    std::uint32_t buf_pos = 0;
};

class FileSource : public Source {
public:
    using Source::Source;

    void drop() override final;

    void pass(Sink& sink) override final;
    void pass(FileSink& sink) override final;

    void copy(Sink& sink) override final;
    void copy(FileSink& sink) override final;
};


class Sink {
public:
    explicit Sink(int fd_ = 1) : fd(fd_) {}

    virtual ~Sink() {}

    template<typename T>
    void send(std::uint16_t id, Packet::Content content, std::vector<T> const& data) {
        send(id, content, 0, data);
    }

    template<typename T>
    void send(std::uint16_t id, Packet::Content content,
              std::uint64_t duration, std::vector<T> const& data) {
        send({ id, content, 0, duration }, data);
    }

    template<typename T>
    void send(Packet pkt, std::vector<T> const& data) {
        send(pkt, data.data(), data.size());
    }

    template<typename T>
    void send(Packet pkt, T const* data, std::uint32_t count = 0) {
        if (count)
            pkt.size = count*sizeof(T);

        send(pkt, reinterpret_cast<std::uint8_t const*>(data));
    }

    virtual void send(Packet pkt, std::uint8_t const* data);

protected:
    friend class Source;
    friend class FileSource;

    int fd = 0;
};

class FileSink : public Sink {
public:
    using Sink::Sink;

    using Sink::send;

    void send(Packet pkt, std::uint8_t const* data) override final;

    friend class Source;
    friend class FileSource;
};


inline void Source::pass(Sink* sink) {
    if (FileSink* fsink = dynamic_cast<FileSink*>(sink))
        pass(*fsink);
    else
        pass(*sink);
}

inline void Source::copy(Sink* sink) {
    if (FileSink* fsink = dynamic_cast<FileSink*>(sink))
        copy(*fsink);
    else
        copy(*sink);
}


bool is_fifo(int fd);

inline bool stdin_is_fifo() {
    return is_fifo(0);
}

inline bool stdout_is_fifo() {
    return is_fifo(1);
}


inline std::unique_ptr<Source> stdin_source() {
    return std::unique_ptr<Source>(stdin_is_fifo() ? new Source(0) : new FileSource(0));
}

inline std::unique_ptr<Sink> stdout_sink() {
    return std::unique_ptr<Sink>(stdout_is_fifo() ? new Sink(1) : new FileSink(1));
}

} /* namespace sdr */
