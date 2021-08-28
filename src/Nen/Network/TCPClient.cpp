#include <SDL_net.h>
#include <vector>
#include <Nen.hpp>

namespace nen
{
    class TCPClient::Impl
    {
    public:
        Impl() = default;
        ~Impl() = default;
        ::IPaddress server_ip;
        ::TCPsocket accepting;
    };

    TCPClient::TCPClient()
        : impl(std::make_unique<Impl>())
    {
    }

    TCPClient::~TCPClient() = default;
    bool TCPClient::ResolveHost(std::string_view address, uint16_t port)
    {
        if (SDLNet_ResolveHost(&impl->server_ip, address.data(), port) != 0)
        {
            Logger::Error("%s", SDLNet_GetError());
            return false;
        }
        return true;
    }

    bool TCPClient::Open()
    {
        impl->accepting = SDLNet_TCP_Open(&impl->server_ip);
        if (impl->accepting != NULL)
        {
            Logger::Error("%s", SDLNet_GetError());
            return false;
        }
        return true;
    }
    void TCPClient::Close()
    {
        SDLNet_TCP_Close(impl->accepting);
    }

    bool TCPClient::Receive(void *data, int maxLength)
    {
        return (SDLNet_TCP_Recv(impl->accepting, data, maxLength) >= 1);
    }

    bool TCPClient::Send(const void *data, int size)
    {
        return SDLNet_TCP_Send(impl->accepting, data, size) >= size;
    }
}
