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
 #include "libsocket/common.hpp"
 #include "libsocket/ssl.hpp"
 #include "libsocket/dns.hpp"
 #include "libsocket/utils.hpp"
 ```

 ---

 ## Examples

 ### TCP Server (IPv4)

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"

 int main() {
     libsocket::descriptor sock = libsocket::ipv4::tcp::socket();
     libsocket::bind(sock, libsocket::ipv4::dns::resolve("localhost", 8000));
     libsocket::listen(sock, 0);

     while (true) {
         libsocket::descriptor client = libsocket::accept(sock);

         std::cout << libsocket::readstring(client, 1024) << std::endl;
         libsocket::writestring(client, "Hello from TCP server!");

         libsocket::shutdown(client);
         libsocket::close(client);
     }

     libsocket::close(sock);
 }
 ```

 ---

 ### TCP Client (IPv4)

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 
 int main() {
     libsocket::descriptor sock = libsocket::ipv4::tcp::socket();
     libsocket::connect(sock, libsocket::ipv4::dns::resolve("localhost", 8000));
 
     libsocket::writestring(sock, "Hi, server!");
     std::cout << libsocket::readstring(sock, 1024) << std::endl;
 
     libsocket::shutdown(sock);
     libsocket::close(sock);
 }
 ```

 ---

 ### UNIX Socket Server

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "unix.hpp"
 
 int main() {
     libsocket::descriptor sock = libsocket::unix::socket();
     libsocket::unix::unlink("test.socket");
     libsocket::bind(sock, {"test.socket"});
     libsocket::listen(sock, 0);
 
     while (true) {
         libsocket::descriptor cl = libsocket::accept(sock);
 
         std::cout << libsocket::readstring(cl, 1024) << std::endl;
         libsocket::writestring(cl, "Hello from UNIX server!");
         
         libsocket::close(cl);
     }
 
     libsocket::close(sock);
 }
 ```

 ---

 ### SSL/TLS Server

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 #include "ssl.hpp"
 
 int main() {
     libsocket::utils::ssl::init();
     libsocket::ssl_ctx ctx = libsocket::utils::ssl::new_server_context();
 
     libsocket::utils::ssl::load_cert(ctx, "server.crt");
     libsocket::utils::ssl::load_key(ctx, "server.key");
 
     libsocket::descriptor sock = libsocket::ipv4::tcp::socket();
     libsocket::bind(sock, libsocket::ipv4::dns::resolve("localhost", 8000));
     libsocket::listen(sock, 0);
 
     while (true) {
         libsocket::descriptor cl = libsocket::accept(sock);
 
         libsocket::ssl::enable(cl, ctx);
         libsocket::ssl::handshake(cl);
 
         std::cout << libsocket::ssl::readstring(cl, 1024) << std::endl;
         libsocket::ssl::writestring(cl, "Hello from SSL server!");
 
         libsocket::ssl::shutdown(cl);
         libsocket::close(cl);
     }
 
     libsocket::close(sock);
 }
 ```

 ---

 ### SSL/TLS Client

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "tcp.hpp"
 #include "dns.hpp"
 #include "ssl.hpp"
 
 int main() {
     libsocket::utils::ssl::init();
     libsocket::ssl_ctx ctx = libsocket::utils::ssl::new_client_context();
 
     libsocket::descriptor sock = libsocket::ipv4::tcp::socket();
     libsocket::connect(sock, libsocket::ipv4::dns::resolve("localhost", 8000));
 
     libsocket::ssl::enable(sock, ctx);
     libsocket::ssl::handshake(sock);
 
     libsocket::ssl::writestring(sock, "Secure hello!");
     std::cout << libsocket::ssl::readstring(sock, 1024) << std::endl;
 
     libsocket::ssl::shutdown(sock);
     libsocket::close(sock);
 }
 ```

---

 ### UDP Server

 ```cpp
 #include <iostream>
 #include <algorithm>
 #include "socket.hpp"
 #include "udp.hpp"
 #include "dns.hpp"
 
 int main() {
     libsocket::descriptor sock = libsocket::ipv4::udp::socket();
     libsocket::bind(sock, libsocket::ipv4::dns::resolve("localhost", 8000));
 
     while (true) {
         libsocket::string_datagram temp = libsocket::readstringfrom(sock, 1024);
 
         std::cout << temp.addr.string() << ": " << temp.data << std::endl;
 
         std::transform(temp.data.begin(), temp.data.end(), temp.data.begin(), ::toupper);
         
         libsocket::writestringto(sock, temp.data, temp.addr);
     }
 
     libsocket::close(sock);
 }
 ```

 ---

 ### UDP Client

 ```cpp
 #include <iostream>
 #include "socket.hpp"
 #include "udp.hpp"
 #include "dns.hpp"
 
 int main() {
     libsocket::descriptor sock = libsocket::ipv4::udp::socket();
     libsocket::address_list addr = libsocket::ipv4::dns::resolve("localhost", 8000);
 
     libsocket::writestringto(sock, "Yay! UDP is working!", addr.front());
 
     libsocket::string_datagram temp = libsocket::readstringfrom(sock, 1024);
 
     std::cout << temp.addr.string() << ": " << temp.data << std::endl;
 
     libsocket::close(sock);
 }
 ```

 ---

 ## DNS Example

 ```cpp
 auto addr = libsocket::ipv4::dns::resolve("example.com", 443);
 ```

 ---

 ## Roadmap

 - UDP support ✅
 - Unified API for both stream and datagram sockets ✅
 - Add more error checking

 ---

 ## License

 MIT License
