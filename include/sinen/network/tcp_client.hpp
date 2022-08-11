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
  bool ResolveHost(std::string_view address, uint16_t port);
  bool Open();
  void Close();

  bool Receive(void *data, int maxLength);
  bool Send(const void *data, int size);

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};
} // namespace sinen
#endif // !SINEN_TCP_CLIENT_HPP
