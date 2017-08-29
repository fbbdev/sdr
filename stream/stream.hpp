#pragma once

#include "packet.hpp"

#include <cstdint>
#include <array>
#include <vector>

namespace sdr
{

bool is_fifo(int fd);

enum RawTag {
    Raw,
};

class Source {
public:
    explicit Source(int fd_ = 0) : fd(fd_), fifo(is_fifo(fd_)) {}

    explicit Source(RawTag, int fd_ = 0)
        : fd(fd_), raw(true), fifo(is_fifo(fd_)) {}

    bool next(Packet rawpkt = {});

    bool poll(int timeout = 0);

    bool end() const noexcept {
        return eof;
    }

    Packet const& packet() const noexcept {
        return pkt;
    }

    template<typename T, typename Alloc = std::allocator<T>>
    std::vector<T, Alloc> recv() {
        if (!pkt.compatible<T>())
            return std::vector<T>();

        std::vector<T> data(pkt.count<T>());

        auto read = recv(data);
        data.resize(read);

        return data;
    }

    template<typename T, typename Alloc>
    std::uint32_t recv(std::vector<T, Alloc>& data) {
        return recv(data.data(), data.size());
    }

    template<typename T>
    std::uint32_t recv(T* data, std::uint32_t count = 0) {
        if (!pkt.compatible<T>())
            return 0;

        return recv(reinterpret_cast<std::uint8_t*>(data), count*sizeof(T))/sizeof(T);
    }

    std::uint32_t recv(std::uint8_t* data, std::uint32_t size = 0);

    void drop();

    void pass(class Sink& sink);
    void copy(class Sink& sink);

protected:
    int fd;
    bool raw = false;
    bool fifo;

    Packet pkt{};
    std::uint32_t read = 0;
    bool eof = false;

    std::array<std::uint8_t, sizeof(Packet)> pkt_buf;
    std::size_t pkt_buf_pos = 0;

    std::vector<std::uint8_t> buffer;
    std::uint32_t buf_pos = 0;
};


class Sink {
public:
    explicit Sink(int fd_ = 1) : fd(fd_), fifo(is_fifo(fd_)) {}

    explicit Sink(RawTag, int fd_ = 1)
        : fd(fd_), raw(true), fifo(is_fifo(fd_))
        {}

    template<typename T, typename Alloc>
    void send(std::uint16_t id, Packet::Content content, std::vector<T, Alloc> const& data) {
        send(id, content, 0, data);
    }

    template<typename T, typename Alloc>
    void send(std::uint16_t id, Packet::Content content,
              std::uint64_t duration, std::vector<T, Alloc> const& data) {
        send({ id, content, 0, duration }, data);
    }

    template<typename T, typename Alloc>
    void send(Packet pkt, std::vector<T, Alloc> const& data) {
        send(pkt, data.data(), data.size());
    }

    template<typename T>
    void send(Packet pkt, T const* data, std::uint32_t count = 0) {
        if (count)
            pkt.size = count*sizeof(T);

        send(pkt, reinterpret_cast<std::uint8_t const*>(data));
    }

    void send(Packet pkt, std::uint8_t const* data);

protected:
    friend class Source;

    int fd = 0;
    bool raw = false;
    bool fifo;
};

} /* namespace sdr */
