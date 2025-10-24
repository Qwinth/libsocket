 # libsocket

 **libsocket** — header-only C++ socket library supporting TCP, UDP, UNIX sockets, and SSL/TLS.
 It is designed for simplicity, clarity, and low-level control without losing elegance.

 ---

 ## Features

 - IPv4, IPv6, and UNIX socket support
 - TCP and UDP protocols
 - SSL/TLS support via OpenSSL
 - Unified API with clear namespace structure
 - `any` namespace for generic stream operations
 - Built-in DNS resolver
 - Simple option handling via `setsockopt`

 ---

 ## Installation

 No external dependencies except OpenSSL (for SSL/TLS).
 Just include the headers in your project:

 ```cpp
 #include "libsocket/socket.hpp"
 #include "libsocket/tcp.hpp"
 #include "libsocket/udp.hpp"
 #include "libsocket/unix.hpp"
 #include "libsocket/ssl.hpp"
 #include "libsocket/dns.hpp"
 #include "libsocket/utils.hpp"
 ```

 ---

 ## Examples

 ### TCP Server (IPv4)

 ```cpp
 #include <iostream>
 #include "libsocket/socket.hpp"
 #include "libsocket/tcp.hpp"
 #include "libsocket/dns.hpp"
 #include "libsocket/utils.hpp"

 int main() {
     auto socket = libsocket::ipv4::tcp::open();
     libsocket::utils::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, 1);
     libsocket::ipv4::tcp::bind(socket, libsocket::ipv4::dns::resolve("localhost", 8000));
     libsocket::any::stream::listen(socket, 0);

     auto client = libsocket::ipv4::tcp::accept(socket);

     std::cout << libsocket::any::stream::readstring(client, 1024) << std::endl;
     libsocket::any::stream::writestring(client, "Hello from server!");

     libsocket::any::stream::shutdown(client);
     libsocket::any::stream::shutdown(socket);
     libsocket::ipv4::tcp::close(client);
     libsocket::ipv4::tcp::close(socket);
 }
 ```

 ---

 ### UNIX Socket Server

 ```cpp
 #include <iostream>
 #include "libsocket/socket.hpp"
 #include "libsocket/unix.hpp"

 int main() {
     auto socket = libsocket::unix::stream::open();
     libsocket::unix::stream::bind(socket, {"unix.socket"});
     libsocket::any::stream::listen(socket, 0);

     auto client = libsocket::unix::stream::accept(socket);

     std::cout << libsocket::any::stream::readstring(client, 1024) << std::endl;
     libsocket::any::stream::writestring(client, "Hello from UNIX server!");

     libsocket::any::stream::shutdown(client);
     libsocket::any::stream::shutdown(socket);
     libsocket::unix::stream::close(client);
     libsocket::unix::stream::close(socket);
 }
 ```

 ---

 ### SSL/TLS Server

 ```cpp
 #include <iostream>
 #include "libsocket/socket.hpp"
 #include "libsocket/tcp.hpp"
 #include "libsocket/ssl.hpp"
 #include "libsocket/dns.hpp"
 #include "libsocket/utils.hpp"

 int main() {
     libsocket::utils::ssl::init();

     auto ctx = libsocket::utils::ssl::new_server_context();
     libsocket::utils::ssl::load_cert(ctx, "server.crt");
     libsocket::utils::ssl::load_key(ctx, "server.key");

     auto socket = libsocket::ipv4::tcp::open();
     libsocket::utils::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, 1);
     libsocket::ipv4::tcp::bind(socket, libsocket::ipv4::dns::resolve("localhost", 8000));
     libsocket::any::stream::listen(socket, 0);

     auto client = libsocket::ipv4::tcp::accept(socket);
     libsocket::any::stream::ssl::enable(client, ctx);
     libsocket::any::stream::ssl::handshake(client);

     std::cout << libsocket::any::stream::ssl::readstring(client, 1024) << std::endl;
     libsocket::any::stream::ssl::writestring(client, "Hello from SSL server!");

     libsocket::any::stream::ssl::shutdown(client);
     libsocket::any::stream::shutdown(socket);
     libsocket::ipv4::tcp::close(client);
     libsocket::ipv4::tcp::close(socket);
 }
 ```

 ---

 ## DNS Example

 ```cpp
 auto addr = libsocket::ipv4::dns::resolve("example.com", 80);
 ```

 ---

 ## Roadmap

 - UDP support
 - Unified API for both stream and datagram sockets
 - Add more error checking

 ---

 ## License

 MIT License
