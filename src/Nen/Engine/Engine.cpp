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
	nextScene = nullptr;
	nen::Window::name = "Nen";
	nen::Logger::MakeLogger(std::move(nen::Logger::NenLoggers::CreateConsoleLogger()));
	std::shared_ptr<nen::Renderer> renderer;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
	std::ifstream ifs("./api");
	std::string str;
	if (ifs.fail())
		return false;
	std::getline(ifs, str);
	if (str.compare("Vulkan") == 0)
		renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::Vulkan);
	else if (str.compare("OpenGL") == 0)
		renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::OpenGL);

#else
	renderer = std::make_shared<nen::Renderer>(nen::GraphicsAPI::ES);
#endif

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
