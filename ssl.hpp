#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <mutex>
#include <atomic>
#include <cstdint>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "def.hpp"
#include "socket.hpp"
#include "address.hpp"
#include "utils.hpp"

namespace libsocket {
    using ssl_ctx = SSL_CTX*;
    using ssl_conn = SSL*;

    std::map<int32_t, ssl_conn> ssl_conn_table;

    std::atomic_bool openssl_init = false;

    namespace utils::ssl {
        void init() {
            SSL_load_error_strings();
            SSL_library_init();
            OpenSSL_add_all_algorithms();

            openssl_init = true;
        }

        ssl_ctx new_server_context() {
            return SSL_CTX_new(SSLv23_server_method());
        }

        ssl_ctx new_client_context() {
            return SSL_CTX_new(SSLv23_client_method());
        }

        void load_cert(ssl_ctx ctx, std::string cert) {
            SSL_CTX_use_certificate_file(ctx, cert.c_str(), SSL_FILETYPE_PEM);
        }

        void load_key(ssl_ctx ctx, std::string key) {
            SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM);
        }

        void load_trust(ssl_ctx ctx, std::string ca_path) {
            SSL_CTX_load_verify_locations(ctx, ca_path.c_str(), nullptr);
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        }

        // void set_verify(ssl_ctx ctx, int32_t verify) {
        //     SSL_CTX_set_verify(ctx, verify, nullptr);
        // }

        void free_context(ssl_ctx ctx) {
            SSL_CTX_free(ctx);
        }
    }

    namespace any::stream::ssl {
        void enable(descriptor desc, ssl_ctx ctx) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("ssl::enable(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);

            ssl_conn ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock.fd);

            ssl_conn_table.try_emplace(desc.id, ssl);
        }

        void handshake(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("ssl::handshake(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);
            ssl_conn ssl = ssl_conn_table.at(desc.id);

            if (sock.accepted) SSL_accept(ssl);
            else SSL_connect(ssl);
        }

        std::vector<int8_t> read(descriptor desc, int64_t size) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("ssl::read(): socket closed");

            ssl_conn ssl = ssl_conn_table.at(desc.id);

            std::vector<int8_t> buffer(size);
            buffer.resize(::SSL_read(ssl, buffer.data(), size));

            return buffer;
        }

        std::string readstring(descriptor desc, int64_t size) {
            std::vector<int8_t> buffer = read(desc, size);

            return std::string(buffer.begin(), buffer.end());
        }

        int64_t write(descriptor desc, std::vector<int8_t> buffer) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("ssl::write(): socket closed");

            ssl_conn ssl = ssl_conn_table.at(desc.id);

            return ::SSL_write(ssl, buffer.data(), buffer.size());
        }

        int64_t writestring(descriptor desc, std::string string) {
            return write(desc, std::vector<int8_t>(string.begin(), string.end()));
        }

        void shutdown(descriptor desc) {
            std::unique_lock lock(socket_table_mutex);

            if (!libsocket::utils::descriptor_ok(desc)) throw std::runtime_error("ssl::shutdown(): socket closed");

            libsocket::utils::socket& sock = socket_table.at(desc.id);
            ssl_conn ssl = ssl_conn_table.at(desc.id);

            SSL_shutdown(ssl);
            SSL_free(ssl);

            ssl_conn_table.erase(desc.id);

            ::shutdown(sock.fd, SHUT_RDWR);

            sock.working = false;
        }
    }
}