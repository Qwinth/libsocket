#pragma once
#include <random>
#include <limits>
#include <stdexcept>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "def.hpp"
#include "socket.hpp"

#undef unix

namespace libsocket {
    namespace utils {
        std::random_device rd; 
        std::mt19937_64 mersenne(rd());

        std::uniform_int_distribution<std::mt19937_64::result_type> random_s32(0, std::numeric_limits<int32_t>::max());
        std::uniform_int_distribution<std::mt19937_64::result_type> random_u64(0, std::numeric_limits<uint64_t>::max());

        bool descriptor_ok(descriptor desc) {
            auto it = socket_table.find(desc.id);

            if (it != socket_table.end()) return desc.fingerprint == it->second.fingerprint;

            return false;
        }

        template<typename T>
        void setsockopt(descriptor desc, int32_t level, int32_t optname, T optval) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("setsockopt(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (::setsockopt(sock.fd, level, optname, &optval, sizeof(T)) == -1) throw std::runtime_error("setsockopt(): Unable to set socket option: " + std::string(strerror(errno)));
        }

        template<typename T>
        int32_t getsockopt(descriptor desc, int32_t level, int32_t optname, T& optval, socklen_t size = sizeof(T)) {
            std::unique_lock lock(socket_table_mutex);

            if (!descriptor_ok(desc)) throw std::runtime_error("getsockopt(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (::getsockopt(sock.fd, level, optname, &optval, &size) == -1) throw std::runtime_error("getsockopt(): Unable to get socket option: " + std::string(strerror(errno)));

            return size;
        }

        address getsockname(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getsockname(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_storage my_addr;
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getsockname(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getsockname(): Unable to get socket name: " + std::string(strerror(errno)));

            return address::from_sockaddr(my_addr);
        }

        address getpeername(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_storage my_addr;
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getpeername(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getpeername(): Unable to get socket name: " + std::string(strerror(errno)));

            return address::from_sockaddr(my_addr);
        }
    }
}
