#pragma once
#include <vector>
#include <cstdint>

#include <sys/un.h>
#include <netinet/in.h>

#include "address.hpp"

#undef unix

namespace libsocket {
    using fd_t = int32_t;
    using socklen_t = uint32_t;
    using address_list = std::vector<address>;

    struct datagram {
        address addr;
        std::vector<int8_t> data;
    };

    struct string_datagram {
        address addr;
        std::string data;
    };
}
