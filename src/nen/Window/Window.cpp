#include <nen.hpp>

namespace nen
{
    Uint32 Window::Info::id = 0;
    ::SDL_SysWMinfo Window::Info::info = SDL_SysWMinfo{};
}