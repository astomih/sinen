#include <SDL_net.h>
#include <network/tcp_socket.hpp>

namespace nen {
class tcp_socket::Impl {
public:
  Impl() = default;
  ~Impl() = default;
};
tcp_socket::tcp_socket() : impl(std::make_unique<Impl>()){};
tcp_socket::~tcp_socket() = default;
} // namespace nen