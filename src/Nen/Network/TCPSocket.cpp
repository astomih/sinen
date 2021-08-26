#include <SDL_net.h>
#include <Nen.hpp>

namespace nen
{
    class TCPSocket::Impl
    {
    public:
        Impl() = default;
        ~Impl() = default;
    };
    TCPSocket::TCPSocket()
        : impl(std::make_unique<Impl>()){};
    TCPSocket::~TCPSocket() = default;
}