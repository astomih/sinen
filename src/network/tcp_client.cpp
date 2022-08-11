#include <SDL_net.h>
#include <logger/logger.hpp>
#include <network/tcp_client.hpp>
#include <vector>

namespace sinen {
class tcp_client::Impl {
public:
  Impl() = default;
  ~Impl() = default;
  ::IPaddress server_ip;
  ::TCPsocket accepting;
};

tcp_client::tcp_client() : impl(std::make_unique<Impl>()) {}

tcp_client::~tcp_client() = default;
bool tcp_client::ResolveHost(std::string_view address, uint16_t port) {
  if (SDLNet_ResolveHost(&impl->server_ip, address.data(), port) != 0) {
    logger::error("%s", SDLNet_GetError());
    return false;
  }
  return true;
}

bool tcp_client::Open() {
  impl->accepting = SDLNet_TCP_Open(&impl->server_ip);
  if (impl->accepting != NULL) {
    logger::error("%s", SDLNet_GetError());
    return false;
  }
  return true;
}
void tcp_client::Close() { SDLNet_TCP_Close(impl->accepting); }

bool tcp_client::Receive(void *data, int maxLength) {
  return (SDLNet_TCP_Recv(impl->accepting, data, maxLength) >= 1);
}

bool tcp_client::Send(const void *data, int size) {
  return SDLNet_TCP_Send(impl->accepting, data, size) >= size;
}
} // namespace sinen
