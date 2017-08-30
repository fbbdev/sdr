#include "stream.hpp"

#include <errno.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <poll.h>
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


bool sdr::is_fifo(int fd) {
    struct stat s{};
    fstat(fd, &s);

    return (s.st_mode & S_IFMT) == S_IFIFO || (s.st_mode & S_IFMT) == S_IFSOCK;
}

bool sdr::is_seekable(int fd) {
    return !(lseek(fd, 0, SEEK_CUR) < 0);
}


bool Source::next(Packet rawpkt) {
    drop();
    read = 0;

    if (eof) {
        pkt = Packet();
        return false;
    }

    if (!raw) {
        auto size = pkt_buf.size() - pkt_buf_pos;
        if (read_all(fd, pkt_buf.data(), size) < size) {
            pkt = Packet();
            eof = true;
            return false;
        }

        pkt = *reinterpret_cast<Packet*>(pkt_buf.data());
        pkt_buf_pos = 0;
    } else {
        pkt = rawpkt;

        if (seekable) {
            auto pos = lseek(fd, 0, SEEK_CUR);
            auto size = lseek(fd, 0, SEEK_END);
            lseek(fd, pos, SEEK_SET);

            if (pos == size) {
                pkt = Packet();
                return false;
            }

            pkt.size = std::uint32_t(std::min(size - pos, ssize_t(rawpkt.size)));
        }
    }

    return true;
}

bool Source::poll(int timeout) {
    if (seekable)
        // seekable fd, data is always available
        return true;

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    if (::poll(&pfd, 1, timeout) > 0) {
        if (!(read < pkt.size) && !raw) {
            // waiting for a packet
            if (pkt_buf_pos == pkt_buf.size())
                return true;

            auto r = ::read(fd, pkt_buf.data() + pkt_buf_pos, pkt_buf.size() - pkt_buf_pos);
            if (r <= 0)
                // EOF, or an error occurred
                return true;

            pkt_buf_pos += r;
            return pkt_buf_pos == pkt_buf.size();
        } else {
            return true;
        }
    }

    return false;
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

    if (seekable) {
        lseek(fd, size, SEEK_CUR);
        read = pkt.size;
    } else {
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
}

void Source::pass(Sink& sink) {
    // Cannot pass packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!sink.raw && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet))) {
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

    if (r < pkt.size) {
        if (fifo || sink.fifo) {
            r += splice_all(fd, sink.fd, pkt.size - r);
        } else if (seekable) {
            r += sendfile_all(fd, sink.fd, pkt.size - r);
        } else {
            buffer.resize(pkt.size - r);
            r += read_all(fd, buffer.data(), buffer.size());
            write_all(fd, buffer.data(), r);
        }
    }

    read = r;

    if (r < pkt.size)
        // Possible error on sink during splice/sendfile
        drop();

    if (!sink.fifo)
        fdatasync(sink.fd);
}

void Source::copy(Sink& sink) {
    // Cannot copy packet if data has been already read
    if (read != 0 || eof)
        return;

    if (!sink.raw && !write_all(sink.fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet)))
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

    if (fifo && sink.fifo) {
        // source and sink are both FIFO, use tee
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
    } else if ((fifo || !seekable) && r < pkt.size) {
        // sink is not FIFO or source is not seekable,
        // buffer data before writing
        auto old = buffer.size();
        auto new_ = pkt.size - r;
        buffer.resize(old + new_);

        r += read_all(fd, buffer.data() + old, new_);
        write_all(sink.fd, buffer.data() + old, new_);

        if (r < pkt.size)
            // EOF, or an error occurred
            buffer.resize(r);
    } else {
        // source is seekable, move data and seek back
        if (sink.fifo)
            r = splice_all(fd, sink.fd, pkt.size - r);
        else
            r = sendfile_all(fd, sink.fd, pkt.size - r);

        lseek(fd, -r, SEEK_CUR);
    }

    if (!sink.fifo)
        fdatasync(sink.fd);
}


void Sink::send(Packet pkt, std::uint8_t const* data) {
    if (!raw)
        write_all(fd, reinterpret_cast<std::uint8_t*>(&pkt), sizeof(Packet));

    write_all(fd, const_cast<std::uint8_t*>(data), pkt.size);

    if (!fifo)
        fdatasync(fd);
}
