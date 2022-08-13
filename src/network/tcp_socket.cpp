#include <SDL_net.h>
#include <network/tcp_socket.hpp>

namespace sinen {
class tcp_socket::impl {
public:
  impl() = default;
  ~impl() = default;
};
tcp_socket::tcp_socket() : m_impl(std::make_unique<impl>()){};
tcp_socket::~tcp_socket() = default;
} // namespace sinen
