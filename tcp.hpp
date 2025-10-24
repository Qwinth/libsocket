#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "stream.hpp"
#include "utils.hpp"

namespace libsocket {
    namespace ipv4::tcp {
        descriptor open() {
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

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("open(tcp|ipv4): Unable to open libsocket::utils::socket");
            }
            
            return {id, fingerprint};
        }

        void connect(descriptor desc, address addr) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (addr.family() != sock.family) throw std::runtime_error("connect(ipv4): Invalid address family");

            utils::connect(sock.fd, addr);

            sock.working = true;
            sock.laddress = libsocket::ipv4::utils::getsockname(desc);
            sock.raddress = addr;
        }

        void connect(descriptor desc, address_list addr_list) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            for (address& addr : addr_list) {
                if (addr.family() != sock.family) continue;

                try { utils::connect(sock.fd, addr); } catch(std::runtime_error) { continue; }

                sock.working = true;
                sock.laddress = libsocket::ipv4::utils::getsockname(desc);
                sock.raddress = addr;

                break;
            }

            if (!sock.working) throw std::runtime_error("connect(ipv4): Unable to connect to host: No route to host");
        }

        void bind(descriptor desc, address addr) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (addr.family() != sock.family) throw std::runtime_error("bind(ipv4): Invalid address family");

            utils::bind(sock.fd, addr);

            sock.working = true;
            sock.laddress = addr;
        }

        void bind(descriptor desc, address_list addr_list) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            for (address& addr : addr_list) {
                if (addr.family() != sock.family) continue;

                try { utils::bind(sock.fd, addr); } catch (std::runtime_error) { continue; }

                sock.working = true;
                sock.laddress = addr;

                break;
            }

            if (!sock.working) throw std::runtime_error("bind(ipv4): Unable to bind to host: No route to host");
        }

        descriptor accept(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("accept(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            accepted client = utils::accept(sock.fd);

            int32_t id = libsocket::utils::random_s32(libsocket::utils::mersenne);
            uint64_t fingerprint = libsocket::utils::random_u64(libsocket::utils::mersenne);

            descriptor new_desc = {id, fingerprint};

            libsocket::utils::socket& new_sock = socket_table.try_emplace(id).first->second;

            new_sock.fd = client.fd;
            new_sock.fingerprint = fingerprint;

            new_sock.working = true;
            new_sock.blocking = true;
            new_sock.listen = false;
            new_sock.accepted = true;

            new_sock.family = AF_INET;
            new_sock.type = SOCK_STREAM;

            new_sock.laddress = libsocket::ipv4::utils::getsockname(new_desc);
            new_sock.raddress = libsocket::ipv4::utils::getpeername(new_desc);

            return new_desc;
        }

        void close(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("close(ipv4): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            ::close(sock.fd);

            socket_table.erase(desc.id);
        }
    }

    namespace ipv6::tcp {
        descriptor open() {
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

            if (sock.fd == -1) {
                socket_table.erase(id);

                throw std::runtime_error("open(tcp|ipv6): Unable to open libsocket::utils::socket");
            }
            
            return {id, fingerprint};
        }

        void connect(descriptor desc, address addr) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (addr.family() != sock.family) throw std::runtime_error("connect(ipv6): Invalid address family");

            utils::connect(sock.fd, addr);

            sock.working = true;
            sock.raddress = addr;
        }

        void connect(descriptor desc, address_list addr_list) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("connect(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            for (address& addr : addr_list) {
                if (addr.family() != sock.family) continue;

                try { utils::connect(sock.fd, addr); } catch(std::runtime_error) { continue; }

                sock.working = true;
                sock.laddress = libsocket::ipv6::utils::getsockname(desc);
                sock.raddress = addr;

                break;
            }

            if (!sock.working) throw std::runtime_error("connect(ipv6): Unable to connect to host: " + std::string(strerror(errno)));
        }

        void bind(descriptor desc, address addr) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            if (addr.family() != sock.family) throw std::runtime_error("connect(ipv6): Invalid address family");

            utils::bind(sock.fd, addr);

            sock.working = true;
            sock.laddress = addr;
        }

        void bind(descriptor desc, address_list addr_list) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("bind(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            for (address& addr : addr_list) {
                if (addr.family() != sock.family) continue;

                try { utils::bind(sock.fd, addr); } catch (std::runtime_error) { continue; }

                sock.working = true;
                sock.laddress = addr;

                break;
            }

            if (!sock.working) throw std::runtime_error("bind(ipv6): Unable to bind to host: " + std::string(strerror(errno)));
        }

        descriptor accept(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("accept(ipv6): socket closed");

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
            new_sock.accepted = true;

            new_sock.family = AF_INET6;
            new_sock.type = SOCK_STREAM;

            return {id, fingerprint};
        }

        void close(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("close(ipv6): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            ::close(sock.fd);

            socket_table.erase(desc.id);
        }
    }
}
