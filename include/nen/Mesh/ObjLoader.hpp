#pragma once
#include "../Render/Renderer.hpp"
#include <memory>
#include <string_view>
namespace nen::mesh
{
    class ObjLoader
    {
    public:
        static bool Load(std::shared_ptr<class Renderer> renderer, std::string_view, std::string_view);
    };
}