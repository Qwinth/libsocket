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
            bool sock_ok = socket_table.find(desc.id) != socket_table.end();
            bool fingerprint_ok = desc.fingerprint == socket_table.at(desc.id).fingerprint;

            return sock_ok && fingerprint_ok;
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
    }

    namespace ipv4::utils {
        address getsockname(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getsockname(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_in my_addr;
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getsockname(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getsockname(ipv4): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }

        address getpeername(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_in my_addr;
            socklen_t addrlen = sizeof(sockaddr_in);

            if (::getpeername(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getpeername(ipv4): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }
    }

    namespace ipv6::utils {
        address getsockname(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getsockname(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_in6 my_addr;
            socklen_t addrlen = sizeof(sockaddr_in6);

            if (::getsockname(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getsockname(ipv6): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }

        address getpeername(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_in6 my_addr;
            socklen_t addrlen = sizeof(sockaddr_in6);

            if (::getpeername(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getpeername(ipv6): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }
    }

    namespace unix::utils {
        address getsockname(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getsockname(unix): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_un my_addr;
            socklen_t addrlen = sizeof(sockaddr_un);

            if (::getsockname(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getsockname(unix): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }

        address getpeername(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("getpeername(unix): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            sockaddr_un my_addr;
            socklen_t addrlen = sizeof(sockaddr_un);

            if (::getpeername(sock.fd, reinterpret_cast<sockaddr*>(&my_addr), &addrlen) == -1) throw std::runtime_error("getpeername(unix): Unable to get socket name: " + std::string(strerror(errno)));

            return my_addr;
        }
    }

    namespace ipv4::tcp::utils {
        void connect(fd_t fd, sockaddr_in addr) {
            if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("connect(ipv4): Unable to connect to host: " + std::string(strerror(errno)));
        }
        

        void bind(fd_t fd, sockaddr_in addr) {
            if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("bind(ipv4): Unable to bind to host: " + std::string(strerror(errno)));
        }

        accepted accept(fd_t fd) {
            sockaddr_in addr;
            socklen_t socklen = sizeof(addr);

            int32_t new_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &socklen);

            if (new_fd == -1) throw std::runtime_error("accept(ipv4): Unable to accept connection: " + std::string(strerror(errno)));

            return {new_fd, addr};
        }
    }

    namespace ipv6::tcp::utils {
        void connect(fd_t fd, sockaddr_in6 addr) {
            if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("connect(ipv6): Unable to connect to host: " + std::string(strerror(errno)));
        }

        void bind(fd_t fd, sockaddr_in6 addr) {
            if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("bind(ipv6): Unable to bind to host: " + std::string(strerror(errno)));
        }

        accepted accept(fd_t fd) {
            sockaddr_in6 addr;
            socklen_t socklen = sizeof(addr);

            int32_t new_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &socklen);

            if (new_fd == -1) throw std::runtime_error("accept(ipv6): Unable to accept connection: " + std::string(strerror(errno)));

            return {new_fd, addr};
        }
    }

    namespace unix::stream::utils {
        void connect(fd_t fd, sockaddr_un addr) {
            if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("connect(unix): Unable to connect to host: " + std::string(strerror(errno)));
        }

        void bind(fd_t fd, sockaddr_un addr) {
            if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) throw std::runtime_error("bind(unix): Unable to bind to host: " + std::string(strerror(errno)));
        }

        accepted accept(fd_t fd) {
            sockaddr_un addr;
            socklen_t socklen = sizeof(addr);

            int32_t new_fd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &socklen);

            if (new_fd == -1) throw std::runtime_error("accept(unix): Unable to accept connection: " + std::string(strerror(errno)));

            return {new_fd, addr};
        }
    }
}
