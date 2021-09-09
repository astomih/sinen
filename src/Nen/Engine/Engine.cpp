#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <Nen.hpp>
#include <fstream>
#include "../Render/RendererHandle.hpp"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

std::function<void()> loop;
void main_loop() { loop(); }
std::shared_ptr<nen::Scene> scene;
std::shared_ptr<nen::Scene> nextScene;
namespace nen
{
	void ChangeScene(std::shared_ptr<Scene> newScene)
	{
		scene->ExitScene();
		nextScene = newScene;
	}
}
int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	SDLNet_Init();
	nextScene = nullptr;
	std::shared_ptr<nen::Window> window = std::make_shared<nen::Window>();
	nen::Logger::MakeLogger(std::move(nen::Logger::NenLoggers::CreateConsoleLogger()));
	std::shared_ptr<nen::Renderer> renderer;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
	std::ifstream ifs("./api");
	std::string str;
	if (ifs.fail())
		return false;
	std::getline(ifs, str);
	if (str.compare("Vulkan") == 0)
	{
		window->Initialize(nen::Vector2(1280, 720), "Nen : Vulkan", nen::GraphicsAPI::Vulkan);
		renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::Vulkan, window);
	}
	else if (str.compare("OpenGL") == 0)
	{
		window->Initialize(nen::Vector2(1280, 720), "Nen : OpenGL", nen::GraphicsAPI::OpenGL);
		renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::OpenGL, window);
	}

#else
	window->Initialize(nen::Vector2(1280, 720), "Nen", nen::GraphicsAPI::ES);
	renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::ES, window);
#endif
	renderer->SetProjectionMatrix(nen::Matrix4::Perspective(nen::Math::ToRadians(90.f), window->Size().x / window->Size().y, 0.01f, 10000.f));

	nen::RendererHandle::SetRenderer(renderer);
	scene = std::make_shared<Main>();
	scene->Initialize();
	loop = [&]
	{
		if (scene->isRunning())
			scene->RunLoop();
		else if (nextScene)
		{
			scene->Shutdown();
			scene = nextScene;
			scene->Initialize();
			nextScene = nullptr;
		}
		else
		{
			scene->Shutdown();
			scene = nullptr;
			renderer->Shutdown();
			renderer = nullptr;
			window = nullptr;
#ifndef EMSCRIPTEN
			std::exit(0);
#else
			emscripten_force_exit(0);
#endif
		}
	};

#if !defined(EMSCRIPTEN)
	while (true)
		loop();
#else
	emscripten_set_main_loop(main_loop, 120, true);
#endif
	return 0;
}
