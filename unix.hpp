#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "stream.hpp"
#include "utils.hpp"

#undef unix

namespace libsocket::unix::stream {
    descriptor open() {
        int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
        uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

        libsocket::utils::socket& sock = socket_table.try_emplace(id).first->second;

        sock.fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        sock.fingerprint = fingerprint;

        sock.working = false;
        sock.blocking = true;
        sock.listen = false;

        sock.family = AF_UNIX;
        sock.type = SOCK_STREAM;

        if (sock.fd == -1) {
            socket_table.erase(id);

            throw std::runtime_error("open(stream|unix): Unable to open libsocket::utils::socket");
        }
        
        return {id, fingerprint};
    }

    void connect(descriptor desc, address addr) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(unix): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("connect(unix): Invalid address family");

        utils::connect(sock.fd, addr);

        sock.working = true;
        sock.raddress = addr;
    }

    void bind(descriptor desc, address addr) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(unix): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("connect(unix): Invalid address family");

        utils::bind(sock.fd, addr);

        sock.working = true;
        sock.laddress = addr;
    }

    descriptor accept(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("accept(unix): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        accepted client = utils::accept(sock.fd);

        int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
        uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

        libsocket::utils::socket& new_sock = socket_table.try_emplace(id).first->second;

        new_sock.fd = client.fd;
        new_sock.fingerprint = fingerprint;

        new_sock.working = true;
        new_sock.blocking = true;
        new_sock.listen = false;

        new_sock.family = AF_UNIX;
        new_sock.type = SOCK_STREAM;

        return {id, fingerprint};
    }

    void close(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("close(unix): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        ::close(sock.fd);

        if (sock.listen) ::unlink(sock.laddress.string().c_str());

        socket_table.erase(desc.id);
    }
}
