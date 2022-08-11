#ifndef SINEN_TCP_SOCKET_HPP
#define SINEN_TCP_SOCKET_HPP
#include <memory>

namespace sinen {
class tcp_socket {
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  tcp_socket();
  ~tcp_socket();
};
} // namespace sinen
#endif // !SINEN_TCP_SOCKET_HPP
