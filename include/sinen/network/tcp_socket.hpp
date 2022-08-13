#ifndef SINEN_TCP_SOCKET_HPP
#define SINEN_TCP_SOCKET_HPP
#include <memory>
namespace sinen {
class tcp_socket {
  class impl;
  std::unique_ptr<impl> m_impl;

public:
  tcp_socket();
  ~tcp_socket();
};
} // namespace sinen
#endif // !SINEN_TCP_SOCKET_HPP
