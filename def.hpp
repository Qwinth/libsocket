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

    namespace ipv4::tcp {
        struct accepted {
            fd_t fd;
            sockaddr_in addr;
        };
    }

    namespace ipv6::tcp {
        struct accepted {
            fd_t fd;
            sockaddr_in6 addr;
        };
    }

    namespace unix::stream {
        struct accepted {
            fd_t fd;
            sockaddr_un addr;
        };
    }

}