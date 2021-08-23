#include <Nen.hpp>
#include <fstream>
#include "../Render/RendererHandle.hpp"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
std::function<void()> loop;
void main_loop() { loop(); }
int main(int argc, char** argv)
{
	nen::Window::name = "test";
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
	std::shared_ptr<nen::Scene> scene = std::make_shared<Main>();
	scene->Initialize();
	loop = [&]
	{
		if (scene->isRunning())
			scene->RunLoop();
		else
		{
			scene->Shutdown();
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
}
