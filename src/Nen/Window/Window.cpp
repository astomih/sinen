#include <SDL.h>
#include <Nen.hpp>
namespace nen
{
    class Window::Impl
    {
    public:
        Impl()
            : window(nullptr)
        {
        }
        ~Impl()
        {
            SDL_DestroyWindow(window);
        }
        ::SDL_Window *window;
    };

    Window::Window()
        : size(Vector2(1280.f, 720.f)), impl(std::make_unique<Window::Impl>())
    {
    }
    Window::~Window() = default;

    ::SDL_Window *Window::GetSDLWindow()
    {
        return impl->window;
    }

    void Window::Initialize(const Vector2 &size, const std::string &name, GraphicsAPI api)
    {
        this->size = size;
        this->name = name;

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
        switch (api)
        {
        case GraphicsAPI::Vulkan:
        {
            impl->window = SDL_CreateWindow(
                std::string(name).c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                static_cast<int>(size.x),
                static_cast<int>(size.y),
                SDL_WINDOW_VULKAN);

            break;
        }
        case GraphicsAPI::OpenGL:
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            impl->window = SDL_CreateWindow(
                std::string(name).c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                static_cast<int>(size.x),
                static_cast<int>(size.y),
                SDL_WINDOW_OPENGL);
            break;
        }
        default:
            break;
        }
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        // Request a color buffer with 8-bits per RGBA channel
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        // Enable double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        impl->window = SDL_CreateWindow(
            std::string(name).c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(size.x),
            static_cast<int>(size.y),
            SDL_WINDOW_OPENGL);
#endif
    }
}