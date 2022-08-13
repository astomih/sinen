#ifndef SINEN_TCP_CLIENT_HPP
#define SINEN_TCP_CLIENT_HPP
#include "tcp_socket.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace sinen {
class tcp_client {
public:
  tcp_client();
  ~tcp_client();
  bool resolve_host(std::string_view address, uint16_t port);
  bool open();
  void close();

  bool receive(void *data, int maxLength);
  bool send(const void *data, int size);

private:
  class impl;
  std::unique_ptr<impl> m_impl;
};
} // namespace sinen
#endif // !SINEN_TCP_CLIENT_HPP
