#include <SDL_net.h>
#include <logger/logger.hpp>
#include <network/tcp.hpp>
#include <vector>

namespace sinen {
class tcp::impl {
public:
  impl() = default;
  ~impl() = default;
  ::IPaddress server_ip;
  ::TCPsocket accepting;
};

tcp::tcp() : m_impl(std::make_unique<impl>()) {}

tcp::~tcp() = default;
bool tcp::resolve_host(std::string_view address, uint16_t port) {
  if (SDLNet_ResolveHost(&m_impl->server_ip, address.data(), port) != 0) {
    logger::error("Resolve Error \"%s\"", SDLNet_GetError());
    return false;
  }
  return true;
}

bool tcp::open() {
  m_impl->accepting = SDLNet_TCP_Open(&m_impl->server_ip);
  if (m_impl->accepting != NULL) {
    logger::error("%s", SDLNet_GetError());
    return false;
  }
  return true;
}
void tcp::close() { SDLNet_TCP_Close(m_impl->accepting); }

bool tcp::receive(void *data, int max_length) {
  return (SDLNet_TCP_Recv(m_impl->accepting, data, max_length) >= 1);
}

bool tcp::send(const void *data, int size) {
  return SDLNet_TCP_Send(m_impl->accepting, data, size) >= size;
}
} // namespace sinen
