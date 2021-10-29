#pragma once
#include "ShaderType.hpp"
#include <string>

namespace nen
{
    class Shader
    {
    public:
        Shader();
        ~Shader() = default;

        std::string vertName;
        std::string fragName;
        bool operator==(const Shader &info) const
        {
            return this->vertName == info.vertName && this->fragName == info.fragName;
        }
    };
}