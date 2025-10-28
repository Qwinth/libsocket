#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>
#include <unistd.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "utils.hpp"

namespace libsocket {
    void connect(descriptor desc, address addr) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("connect(): Invalid address family");

        sockaddr_storage tmp_addr = addr;

        if (::connect(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("connect(): Unable to connect to host: " + std::string(strerror(errno)));

        sock.working = true;
        sock.laddress = libsocket::utils::getsockname(desc);
        sock.raddress = addr;
    }

    void connect(descriptor desc, address_list addr_list) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        for (address& addr : addr_list) {
            if (addr.family() != sock.family) continue;
            
            sockaddr_storage tmp_addr = addr;

            if (::connect(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) continue;

            sock.working = true;
            sock.laddress = libsocket::utils::getsockname(desc);
            sock.raddress = addr;

            break;
        }

        if (!sock.working) throw std::runtime_error("connect(): Unable to connect to host: " + std::string(strerror(errno)));
    }

    void bind(descriptor desc, address addr) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        if (addr.family() != sock.family) throw std::runtime_error("bind(): Invalid address family");

        sockaddr_storage tmp_addr = addr;

        if (::bind(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("bind(): Unable to connect to host: " + std::string(strerror(errno)));

        sock.working = true;
        sock.laddress = addr;
    }

    void bind(descriptor desc, address_list addr_list) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        for (address& addr : addr_list) {
            if (addr.family() != sock.family) continue;

            sockaddr_storage tmp_addr = addr;

            if (::bind(sock.fd, reinterpret_cast<sockaddr*>(&tmp_addr), sock.sockaddr_size) == -1) throw std::runtime_error("bind(): Unable to connect to host: " + std::string(strerror(errno)));

            sock.working = true;
            sock.laddress = addr;

            break;
        }

        if (!sock.working) throw std::runtime_error("bind(): Unable to bind to host: "  + std::string(strerror(errno)));
    }

    descriptor accept(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("accept(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        sockaddr_storage addr;
        socklen_t socklen = sock.sockaddr_size;

        int32_t new_fd = ::accept(sock.fd, reinterpret_cast<sockaddr*>(&addr), &socklen);

        if (new_fd == -1) throw std::runtime_error("accept(): Unable to accept connection: " + std::string(strerror(errno)));

        int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
        uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

        descriptor new_desc = {id, fingerprint};

        libsocket::utils::socket& new_sock = socket_table.try_emplace(id).first->second;

        new_sock.fd = new_fd;
        new_sock.fingerprint = fingerprint;

        new_sock.working = true;
        new_sock.blocking = true;
        new_sock.listen = false;
        new_sock.accepted = true;

        new_sock.family = AF_INET;
        new_sock.type = SOCK_STREAM;
        new_sock.sockaddr_size = sock.sockaddr_size;

        new_sock.laddress = libsocket::utils::getsockname(new_desc);
        new_sock.raddress = address::from_sockaddr(addr);

        return new_desc;
    }

    void listen(descriptor desc, int32_t __listen) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("listen(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        if (::listen(sock.fd, __listen) == -1) throw std::runtime_error("listen(): Unable to listen to host: " + std::string(strerror(errno)));

        sock.listen = true;
    }

    std::vector<int8_t> read(descriptor desc, int64_t size, int32_t flags = 0) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        std::vector<int8_t> buffer(size);
        buffer.resize(::recv(sock.fd, buffer.data(), size, flags));

        return buffer;
    }

    std::string readstring(descriptor desc, int64_t size, int32_t flags = 0) {
        std::vector<int8_t> buffer = read(desc, size, flags);

        return std::string(buffer.begin(), buffer.end());
    }

    int64_t write(descriptor desc, std::vector<int8_t> buffer, int32_t flags = 0) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        return ::send(sock.fd, buffer.data(), buffer.size(), flags);
    }

    int64_t writestring(descriptor desc, std::string string, int32_t flags = 0) {
        return write(desc, std::vector<int8_t>(string.begin(), string.end()), flags);
    }

    datagram readfrom(descriptor desc, int64_t size, int32_t flags = 0) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        sockaddr_storage tmp_addr;
        socklen_t socklen = sock.sockaddr_size;

        std::vector<int8_t> buffer(size);
        buffer.resize(::recvfrom(sock.fd, buffer.data(), size, flags, reinterpret_cast<sockaddr*>(&tmp_addr), &socklen));

        return {address::from_sockaddr(tmp_addr), buffer};
    }

    string_datagram readstringfrom(descriptor desc, int64_t size, int32_t flags = 0) {
        datagram read = readfrom(desc, size, flags);

        return {read.addr, std::string(read.data.begin(), read.data.end())};
    }

    int64_t writeto(descriptor desc, std::vector<int8_t> buffer, address addr, int32_t flags = 0) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("read(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        sockaddr_storage tmp_addr = addr;

        return ::sendto(sock.fd, buffer.data(), buffer.size(), flags, reinterpret_cast<sockaddr*>(&tmp_addr), sizeof(tmp_addr));
    }

    int64_t writestringto(descriptor desc, std::string string, address addr, int32_t flags = 0) {
        return writeto(desc, std::vector<int8_t>(string.begin(), string.end()), addr, flags);
    }

    void shutdown(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("shutdown(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        ::shutdown(sock.fd, SHUT_RDWR);

        sock.working = false;
    }

    void close(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("close(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        ::close(sock.fd);

        socket_table.erase(desc.id);
    }
}
