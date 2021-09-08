#pragma once
#include <memory>
namespace nen
{
    class WindowState
    {
    public:
        WindowState();
        ~WindowState();

        void ProcessInput(union SDL_Event &event);
    };
}