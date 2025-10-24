#pragma once
#include <string>
#include <cstdint>
#include <cstring>

#include <sys/un.h>
#include <netinet/in.h>

namespace libsocket {
    class address {
        union {
            uint8_t raw_addr[sizeof(sockaddr_un::sun_path)];

            uint32_t ip_v4;
            uint32_t ip_v6[sizeof(in6_addr) / sizeof(uint32_t)];
        } __addr;

        uint16_t __port;
        uint16_t __family;
    public:
        address() {}
        // address(SAType __type) : __addr_type(__type) {}
        address(sockaddr_in addr) {
            __addr.ip_v4 = addr.sin_addr.s_addr;
            __port = addr.sin_port;
            __family = addr.sin_family;
        }

        address(sockaddr_in6 addr) {
            __port = addr.sin6_port;
            __family = addr.sin6_family;

            memcpy(__addr.raw_addr, addr.sin6_addr.__in6_u.__u6_addr8, sizeof(in6_addr));
        }

        address(sockaddr_un addr) {
            __family = addr.sun_family;

            memcpy(__addr.raw_addr, addr.sun_path, sizeof(sockaddr_un::sun_path));
        }

        address(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint16_t p) : __port(p), __family(AF_INET) {
            __addr.ip_v4 = a1 | (a2 << 8) | (a3 << 16) | (a4 << 24);
        }

        address(uint32_t addr) : __family(AF_INET) {
            __addr.ip_v4 = addr;
        }

        address(uint32_t addr, uint16_t p) : __port(p), __family(AF_INET) {
            __addr.ip_v4 = addr;
        }

        address(std::string unix_addr) : __family(AF_UNIX) {

            // std::copy(unix_addr.begin(), unix_addr.end(), __addr.raw_addr);
            memset(__addr.raw_addr, 0, sizeof(sockaddr_un::sun_path));
            memcpy(__addr.raw_addr, unix_addr.c_str(), std::min<uint16_t>(unix_addr.size(), sizeof(sockaddr_un::sun_path)));
        }

        address& operator=(sockaddr_in addr) {
            __addr.ip_v4 = addr.sin_addr.s_addr;
            __port = addr.sin_port;
            __family = addr.sin_family;

            return *this;
        }

        address& operator=(sockaddr_in6 addr) {
            __port = addr.sin6_port;
            __family = addr.sin6_family;

            memcpy(__addr.raw_addr, addr.sin6_addr.__in6_u.__u6_addr8, sizeof(in6_addr));

            return *this;
        }

        address& operator=(sockaddr_un addr) {
            __family = addr.sun_family;

            memcpy(__addr.raw_addr, addr.sun_path, sizeof(sockaddr_un::sun_path));
            
            return *this;
        }

        address& operator=(uint32_t addr) {
            __addr.ip_v4 = addr;

            return *this;
        }

        address& operator=(std::string unix_addr) {

            // std::copy(unix_addr.begin(), unix_addr.end(), __addr.raw_addr);
            memset(__addr.raw_addr, 0, sizeof(sockaddr_un::sun_path));
            memcpy(__addr.raw_addr, unix_addr.c_str(), std::min<uint16_t>(unix_addr.size(), sizeof(sockaddr_un::sun_path)));

            return *this;
        }

        void family(int16_t f) {
            __family = f;
        }

        int16_t family() {
            return __family;
        }

        void port(int16_t p) {
            __port = p;
        }

        int16_t port() {
            return __port;
        }

        uint32_t IPv4() {
            return (__family == AF_INET) ? __addr.ip_v4 : 0;
        }

        sockaddr_in addrInet() {
            if (__family != AF_INET) return {0};

            sockaddr_in addr{};
            addr.sin_addr.s_addr = __addr.ip_v4;
            addr.sin_port = htons(__port);
            addr.sin_family = __family;

            return addr;
        }

        sockaddr_in6 addrInet6() {
            if (__family != AF_INET6) return {0};

            sockaddr_in6 addr{};
            memcpy(addr.sin6_addr.__in6_u.__u6_addr8, __addr.ip_v6, sizeof(in6_addr));
            addr.sin6_port = htons(__port);
            addr.sin6_family = __family;

            return addr;
        }
        
        sockaddr_un addrUnix() {
            if (__family != AF_UNIX) return {0};

            sockaddr_un addr{};
            addr.sun_family = __family;

            memcpy(addr.sun_path, __addr.raw_addr, sizeof(sockaddr_un::sun_path));

            return addr;
        }

        

        std::string string() {
            std::string str_addr;

            if (__family == AF_INET) {
                uint32_t addr = ntohl(__addr.ip_v4);

                str_addr = std::to_string((addr >> 24) & 0xFF) + "." +
                        std::to_string((addr >> 16) & 0xFF) + "." +
                        std::to_string((addr >> 8) & 0xFF) + "." +
                        std::to_string(addr & 0xFF) + ":" +
                        std::to_string(__port);
            }

            else if(__family == AF_INET6) {}
            else if(__family == AF_UNIX) str_addr = std::string(reinterpret_cast<char*>(__addr.raw_addr), sizeof(sockaddr_un::sun_path));

            return str_addr;
        }

        operator uint32_t() {
            return IPv4();
        }

        operator sockaddr_in() {
            return addrInet();
        }

        operator sockaddr_in6() {
            return addrInet6();
        }

        operator sockaddr_un() {
            return addrUnix();
        }

        operator std::string() {
            return string();
        }
    };
};
