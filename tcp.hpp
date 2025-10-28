#pragma once
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>

#include "socket.hpp"
#include "address.hpp"
#include "common.hpp"
#include "utils.hpp"

namespace libsocket {
    namespace ipv4::tcp {
        descriptor socket() {
            int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
            uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

            libsocket::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_INET, SOCK_STREAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;
            sock.accepted = false;

            sock.family = AF_INET;
            sock.type = SOCK_STREAM;
            sock.sockaddr_size = sizeof(sockaddr_in);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(tcp|ipv4): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }

    namespace ipv6::tcp {
        descriptor socket() {
            int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
            uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

            libsocket::utils::socket& sock = socket_table.try_emplace(id).first->second;

            sock.fd = ::socket(AF_INET6, SOCK_STREAM, 0);
            sock.fingerprint = fingerprint;

            sock.working = false;
            sock.blocking = true;
            sock.listen = false;
            sock.accepted = false;

            sock.family = AF_INET6;
            sock.type = SOCK_STREAM;
            sock.sockaddr_size = sizeof(sockaddr_in6);

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("socket(tcp|ipv6): Unable to open socket");
            }
            
            return {id, fingerprint};
        }
    }
}
