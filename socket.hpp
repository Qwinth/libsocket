#pragma once
#include <map>
#include <mutex>
#include <atomic>
#include <cstdint>

#include "def.hpp"
#include "address.hpp"

namespace libsocket {
    namespace utils {
        struct socket {
            fd_t fd;

            uint64_t fingerprint;

            std::atomic_bool working;
            std::atomic_bool blocking;
            std::atomic_bool listen;
            std::atomic_bool accepted;

            int32_t family;
            int32_t type;
            int32_t sockaddr_size;

            address laddress;
            address raddress;

            std::recursive_mutex recvMtx;
            std::recursive_mutex sendMtx;
        };
    }

    struct descriptor {
        int32_t id;
        uint64_t fingerprint;
    };

    std::map<int32_t, utils::socket> socket_table;
    std::recursive_mutex socket_table_mutex;
}
