#include <nen/nen.hpp>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <memory>

class TestScene : public nen::Scene
{
public:
    TestScene() {}
    void Update(float deltaTime) override
    {
    }

private:
    void LoadData() override
    {
        auto font = std::make_shared<nen::Font>();

        if (!font->Load("Assets/Font/SoukouMincho-Font/SoukouMincho.ttf"))
        {
            //Do something
        }
        auto fontActor = std::make_shared<nen::Actor>(*this);
        auto fontcomp = std::make_shared<nen::FontComponent>(*fontActor);
        fontcomp->SetFont(font);
        fontcomp->SetString("Hello World!", nen::Color::White, 30);
        fontActor->AddComponent(fontcomp);
        this->AddActor(fontActor);
    }
};
std::function<void()> loop;
void main_loop() { loop(); }
int main(int argc, char **argv)
{
    nen::Window::name = "test";
    std::shared_ptr<nen::Renderer> renderer;

    try
    {
#ifndef EMSCRIPTEN
        renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::Vulkan);
#else
        renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::ES);
#endif
    }
    catch (const std::bad_alloc &e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    auto scene = std::make_shared<TestScene>();
    scene->Initialize(renderer);

    loop = [&]
    {
        if (scene->isRunning())
        {
            scene->RunLoop();
        }
        else
            scene->Shutdown();
    };

#ifndef EMSCRIPTEN
    while (true)
        loop();
#else
    emscripten_set_main_loop(main_loop, 120, true);
#endif
    return 0;
}
