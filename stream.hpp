#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "utils.hpp"

namespace libsocket::any::stream {
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

    void shutdown(descriptor desc) {
        std::unique_lock lock(socket_table_mutex);

        if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("shutdown(): socket closed");

        libsocket::utils::socket& sock = socket_table.at(desc.id);

        ::shutdown(sock.fd, SHUT_RDWR);

        sock.working = false;
    }
}