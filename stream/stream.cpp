#include "stream.hpp"

#include <errno.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstddef>
#include <algorithm>
#include <iostream>

using namespace sdr;

static int devnull = open("/dev/null", O_WRONLY);

static std::size_t read_all(int fd, std::uint8_t* data, std::size_t size) {
    auto p = data, end = data + size;

    ssize_t r = 0;

    do {
        r = read(fd, p, end - p);
        if (r < 0)
            break;

        p += r;
    } while (p != end && r != 0);

    return std::size_t(p - data);
}

static std::size_t splice_all(int src, int dst, std::size_t size) {
    std::size_t spliced = 0;

    ssize_t s = 0;

    do {
        s = splice(src, NULL, dst, NULL, size - spliced, SPLICE_F_MOVE);
        if (s < 0)
            break;

        spliced += s;
    } while (spliced < size && s != 0);

    return spliced;
}

static std::size_t sendfile_all(int src, int dst, std::size_t size) {
    std::size_t sent = 0;

    ssize_t s = 0;

    do {
        s = sendfile(dst, src, NULL, size - sent);
        if (s < 0) {
            if (errno == EOVERFLOW) {
                --size;
                continue;
            } else {
                break;
            }
        }

        sent += s;
    } while (sent < size);

    return sent;
}

static bool write_all(int fd, std::uint8_t const* data, std::size_t size) {
    auto p = data, end = data + size;

    ssize_t w = 0;

    do {
        w = write(fd, p, end - p);
        if (w < 0)
            break;

        p += w;
    } while (p != end && w > 0);

    return (p == end);
}


bool Source::next() {
    std::uint8_t pkt_data[sizeof(Packet)];

    if (eof) {
        pkt = Packet();
        return false;
    }

    drop();
    read = 0;

    if (read_all(fd, pkt_data, sizeof(Packet)) < sizeof(Packet)) {
        pkt = Packet();
        eof = true;
        return false;
    }

    pkt = *reinterpret_cast<Packet*>(pkt_data);

    return true;
}

std::uint32_t Source::recv(std::uint8_t* data, std::uint32_t size) {
    if (size == 0)
        size = pkt.size;

    size = std::min(size, pkt.size - read);

    if (size == 0 || eof)
        return 0;

    ssize_t r = 0;

    if (buf_pos != buffer.size()) {
        r = std::min(ssize_t(size), ssize_t(buffer.size() - buf_pos));
        data = std::copy_n(buffer.begin() + buf_pos, r, data);
        buf_pos += r;
    }

    if (r < size) {
        r += read_all(fd, data, size - r);

        if (r < size)
            eof = true;
    }

    read += r;

    return std::uint32_t(r);
}

void Source::drop() {
    auto size = pkt.size - read;

    if (size == 0 || eof) {
        buf_pos = 0;
        buffer.resize(0);
        return;
    }

    ssize_t r = 0;

    if (buf_pos != buffer.size()) {
        r = buffer.size() - buf_pos;
        buf_pos = 0;
        buffer.resize(0);
    }

    if (r < size) {
        r += splice_all(fd, devnull, size - r);

        if (r < size)
            eof = true;
    }

    read += r;
}

void Source::pass(Sink& sink, bool dump) {
    // Cannot pass packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet))) {
        // Error on sink
        drop();
        return;
    }

    ssize_t r = 0;

    if (buffer.size()) {
        if (!write_all(sink.fd, buffer.data(), buffer.size())) {
            // Error on sink
            drop();
            return;
        }

        r = buffer.size();
        buf_pos = 0;
        buffer.resize(0);
    }

    if (r < pkt.size)
        r += splice_all(fd, sink.fd, pkt.size - r);

    read = r;

    if (r < pkt.size)
        // Possible error on sink during splice
        drop();
}

void Source::pass(FileSink& sink, bool dump) {
    // Cannot pass packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t const*>(&pkt), sizeof(Packet))) {
        // Error on sink
        drop();
        return;
    }

    ssize_t r = 0;

    if (buffer.size()) {
        if (!write_all(sink.fd, buffer.data(), buffer.size())) {
            // Error on sink
            drop();
            return;
        }

        r = buffer.size();
        buf_pos = 0;
        buffer.resize(0);
    }

    if (r < pkt.size)
        r += splice_all(fd, sink.fd, pkt.size - r);

    read = r;

    if (r < pkt.size)
        // Possible error on sink during splice
        drop();

    fdatasync(sink.fd);
}

void Source::copy(Sink& sink, bool dump) {
    // Cannot copy packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet)))
        // Error on sink
        return;

    ssize_t r = 0;

    if (buffer.size()) {
        if (!write_all(sink.fd, buffer.data(), buffer.size()))
            // Error on sink
            return;

        r = buffer.size();
    }

    buf_pos = 0;

    while (r < pkt.size) {
        ssize_t copied = tee(fd, sink.fd, pkt.size - r, 0);

        if (copied <= 0)
            // EOF, or an error occurred
            return;

        r += copied;

        if (r < pkt.size) {
            auto old = buffer.size();
            buffer.resize(old + copied);

            if (read_all(fd, buffer.data() + old, copied) < std::size_t(copied))
                // Error on source
                return;
        }
    }
}

void Source::copy(FileSink& sink, bool dump) {
    // Cannot copy packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t const*>(&pkt), sizeof(Packet)))
        // Error on sink
        return;

    std::size_t r = 0;

    if (buffer.size()) {
        if (!write_all(sink.fd, buffer.data(), buffer.size()))
            // Error on sink
            return;

        r = buffer.size();
    }

    buf_pos = 0;

    if (r < pkt.size) {
        auto old = buffer.size();
        auto new_ = pkt.size - r;
        buffer.resize(old + new_);

        r += read_all(fd, buffer.data() + old, new_);
        write_all(sink.fd, buffer.data() + old, new_);

        if (r < buffer.size())
            // EOF, or an error occurred
            buffer.resize(r);
    }

    fdatasync(sink.fd);
}


void FileSource::drop() {
    lseek(fd, pkt.size - read, SEEK_CUR);
    read = pkt.size;
}

void FileSource::pass(Sink& sink, bool dump) {
    // Cannot pass packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet))) {
        // Error on sink
        drop();
        return;
    }

    read = splice_all(fd, sink.fd, pkt.size);

    if (read < pkt.size)
        // Possible error on sink during splice
        drop();
}

void FileSource::pass(FileSink& sink, bool dump) {
    // Cannot pass packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t const*>(&pkt), sizeof(Packet))) {
        // Error on sink
        drop();
        return;
    }

    read = sendfile_all(fd, sink.fd, pkt.size);

    if (read < pkt.size)
        // Possible error on sink during splice
        drop();

    fdatasync(sink.fd);
}

void FileSource::copy(Sink& sink, bool dump) {
    // Cannot copy packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet)))
        // Error on sink
        return;

    auto sent = splice_all(fd, sink.fd, pkt.size);
    lseek(fd, -sent, SEEK_CUR);
}

void FileSource::copy(FileSink& sink, bool dump) {
    // Cannot copy packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!dump && !write_all(sink.fd, reinterpret_cast<std::uint8_t const*>(&pkt), sizeof(Packet)))
        // Error on sink
        return;

    auto sent = sendfile_all(fd, sink.fd, pkt.size);
    lseek(fd, -sent, SEEK_CUR);
    fdatasync(sink.fd);
}


void Sink::send(Packet pkt, std::uint8_t const* data) {
    write_all(fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet));
    write_all(fd, const_cast<std::uint8_t*>(data), pkt.size);
}

void FileSink::send(Packet pkt, std::uint8_t const* data) {
    write_all(fd, reinterpret_cast<std::uint8_t const*>(&pkt), sizeof(Packet));
    write_all(fd, data, pkt.size);
    fdatasync(fd);
}


bool sdr::is_fifo(int fd) {
    struct stat s{};
    fstat(fd, &s);

    return s.st_mode & S_IFIFO;
}
