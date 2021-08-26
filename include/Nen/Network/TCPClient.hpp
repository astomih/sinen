#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "TCPSocket.hpp"

namespace nen
{
    class TCPClient
    {
    public:
        TCPClient();
        ~TCPClient();
        bool ResolveHost(std::string_view address, uint16_t port);
        bool Open();
        void Close();

        bool Receive(void *data, int maxLength);
        bool Send(const void *data, int size);

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}