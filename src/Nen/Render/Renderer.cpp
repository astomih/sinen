#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include "Vulkan/VKRenderer.h"
#include "OpenGL/GLRenderer.h"
#include "OpenGLES/ESRenderer.h"
#include <Nen.hpp>
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include "OpenGLES/EffectManagerES.h"
#endif
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "Vulkan/EffectManagerVK.h"
#include "OpenGL/EffectManagerGL.h"
#endif
#include "RendererHandle.hpp"

namespace nen
{
	std::shared_ptr<Renderer> RendererHandle::mRenderer = nullptr;
	Renderer::Renderer(GraphicsAPI api)
		: transPic(nullptr), mScene(nullptr), mWindow(nullptr),
		  renderer(nullptr),
		  RendererAPI(api)
	{
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
		switch (RendererAPI)
		{
		case GraphicsAPI::Vulkan:
			renderer = std::make_unique<vk::VKRenderer>();
			SDL_Init(SDL_INIT_EVERYTHING);
			TTF_Init();
			IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
			mWindow = SDL_CreateWindow(
				std::string(Window::name + " : Vulkan").c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				static_cast<int>(Window::Size.x),
				static_cast<int>(Window::Size.y),
				SDL_WINDOW_VULKAN);
			renderer->Initialize(mWindow);
			renderer->SetRenderer(this);
			break;
		case GraphicsAPI::OpenGL:
			renderer = std::make_unique<gl::GLRenderer>();
			SDL_Init(SDL_INIT_EVERYTHING);
			TTF_Init();
			IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
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
			mWindow = SDL_CreateWindow(
				std::string(Window::name + " : OpenGL").c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				static_cast<int>(Window::Size.x),
				static_cast<int>(Window::Size.y),
				SDL_WINDOW_OPENGL);

			renderer->SetRenderer(this);
			renderer->Initialize(mWindow);
			break;
		default:
			break;
		}
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
		renderer = std::make_unique<es::ESRenderer>();
		SDL_Init(SDL_INIT_EVERYTHING);
		if (TTF_Init() == -1)
			std::cout << "SDL2_TTF failed initialize." << std::endl;
		IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
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
		mWindow = SDL_CreateWindow(
			std::string(Window::name + " : ES").c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			static_cast<int>(Window::Size.x),
			static_cast<int>(Window::Size.y),
			SDL_WINDOW_OPENGL);
		renderer->SetRenderer(this);
		renderer->Initialize(mWindow);
#endif
		if (SDLNet_Init() != 0)
			std::cout << "net init error." << std::endl;
	}

	bool Renderer::Initialize(std::shared_ptr<Scene> scene, std::shared_ptr<Transition> transition)
	{
		return true;
	}

	void Renderer::Shutdown()
	{
		renderer->Shutdown();
	}

	void Renderer::UnloadData()
	{
		// Destroy textures
		for (auto i : mTextures3D)
		{
			delete i.second;
			i.second = nullptr;
		}
		mTextures3D.clear();
	}

	void Renderer::Draw()
	{
		renderer->Render();
	}

	void Renderer::AddDrawObject2D(std::shared_ptr<nen::DrawObject> drawObject, std::shared_ptr<Texture> texture)
	{
		renderer->AddDrawObject2D(drawObject, texture);
		const auto myDrawOrder = drawObject->drawOrder;
		auto iter = mDrawObject2D.begin();
		for (; iter != mDrawObject2D.end(); ++iter)
		{
			if (myDrawOrder < (*iter)->drawOrder)
			{
				break;
			}
		}
		mDrawObject2D.insert(iter, drawObject);
	}

	void Renderer::RemoveDrawObject2D(std::shared_ptr<DrawObject> drawObject)
	{
		renderer->RemoveDrawObject2D(drawObject);
		auto iter = std::find(mDrawObject2D.begin(), mDrawObject2D.end(), drawObject);
		if (iter != mDrawObject2D.end())
			mDrawObject2D.erase(iter);
	}

	void Renderer::AddDrawObject3D(std::shared_ptr<DrawObject> drawObject, std::shared_ptr<Texture> texture)
	{
		renderer->AddDrawObject3D(drawObject, texture);
		// Find the insertion point in the sorted vector
		// (The first element with a higher draw order than me)
		const auto myDrawOrder = drawObject->drawOrder;
		auto iter = mDrawObject3D.begin();
		for (;
			 iter != mDrawObject3D.end();
			 ++iter)
		{
			if (myDrawOrder < (*iter)->drawOrder)
			{
				break;
			}
		}

		// Inserts element before position of iterator
		mDrawObject3D.insert(iter, drawObject);
	}

	void Renderer::RemoveDrawObject3D(std::shared_ptr<DrawObject> drawObject)
	{
		renderer->RemoveDrawObject3D(drawObject);
		auto iter = std::find(mDrawObject3D.begin(), mDrawObject3D.end(), drawObject);
		if (iter != mDrawObject3D.end())
			mDrawObject3D.erase(iter);
	}

	void Renderer::ChangeBufferDrawObject(std::shared_ptr<DrawObject> drawObject, TextureType type)
	{
		renderer->ChangeBufferDrawObject(drawObject, type);
	}

	void Renderer::AddEffect(Effect *effect)
	{
		mEffects.emplace_back(effect);
	}
	void Renderer::RemoveEffect(Effect *effect)
	{
		auto iter = std::find(mEffects.begin(), mEffects.end(), effect);
		mEffects.erase(iter);
	}

	void Renderer::AddGUI(std::shared_ptr<UIScreen> ui)
	{
		renderer->AddGUI(ui);
	}

	void Renderer::RemoveGUI(std::shared_ptr<UIScreen> ui)
	{
		renderer->RemoveGUI(ui);
	}

	Texture *Renderer::GetTexture(std::string_view fileName)
	{
		Texture *tex = nullptr;
		auto iter = mTextures3D.find(fileName.data());
		if (iter != mTextures3D.end())
		{
			tex = iter->second;
		}
		else
		{
			tex = new Texture();
			if (tex->Load(fileName))
			{
				mTextures3D.emplace(fileName, tex);
			}
			else
			{
				delete tex;
				tex = nullptr;
			}
		}
		return tex;
	}

	void Renderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
	{
		renderer->AddVertexArray(vArray, name);
	}

	Texture *Renderer::GetTextureFromMemory(const unsigned char *const buffer, const std::string &key)
	{
		Texture *tex = nullptr;
		auto iter = mTextures3D.find(key);
		if (iter != mTextures3D.end())
		{
			tex = iter->second;
		}
		else
		{
			tex = new Texture();
			/*
			//if (tex->LoadFromMemory(buffer))
			{
				mTextures3D.emplace(key, tex);
			}
			//else
			{
				delete tex;
				tex = nullptr;
			}
			*/
		}
		return tex;
	}
}
