#pragma once
#include <memory>

namespace nen
{
    class TCPSocket
    {
        class Impl;
        std::unique_ptr<Impl> impl;
    public:
        TCPSocket();
        ~TCPSocket();

    };
}