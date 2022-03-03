#include <Network/TCPSocket.hpp>
#include <SDL_net.h>

namespace nen {
class tcp_socket::Impl {
public:
  Impl() = default;
  ~Impl() = default;
};
tcp_socket::tcp_socket() : impl(std::make_unique<Impl>()){};
tcp_socket::~tcp_socket() = default;
} // namespace nen