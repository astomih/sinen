#include <Nen.hpp>
#include <fstream>
#include "../Render/RendererHandle.hpp"
void Setup();
void Update();

std::function<void()> loop;
void main_loop() { loop(); }
int main(int argc, char **argv)
{
    nen::Window::name = /*"刹那の砲撃"*/ "test";
    nen::Logger::MakeLogger(std::move(nen::Logger::NenLoggers::CreateConsoleLogger()));
    nen::Logger::Info("initialize...");
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
    std::ifstream ifs("./api");
    std::string str;
    if (ifs.fail())
        return false;
    std::getline(ifs, str);
    std::shared_ptr<nen::Renderer> renderer;
    if (str.compare("Vulkan") == 0)
        renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::Vulkan);
    else if (str.compare("OpenGL") == 0)
        renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::OpenGL);

#else
    renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::ES);
#endif

    nen::RendererHandle::SetRenderer(renderer);

    Setup();
    loop = [&]
    {
        Update();
    };

#if !defined(EMSCRIPTEN)
    while (true)
        loop();
#else
    emscripten_set_main_loop(main_loop, 120, true);
#endif
    return 0;
}
