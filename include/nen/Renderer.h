#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#ifdef EMSCRIPTEN
#include "ES/ESRenderer.h"
#endif
#include "Math.hpp"
#include "Window.hpp"
#include "Vulkan/VKRenderer.h"
#include "OpenGL/GLRenderer.h"

namespace nen
{

	enum class GraphicsAPI
	{
		OpenGL,
		Vulkan,
		ES
	};

	class Renderer
	{
	public:
		Renderer(GraphicsAPI api);
		~Renderer() = default;

		void SetGraphicsAPI(GraphicsAPI &api)
		{
			RendererAPI = api;
		}
		GraphicsAPI GetGraphicsAPI()
		{
			return RendererAPI;
		}

		bool Initialize(std::shared_ptr<class Scene> scene, std::shared_ptr<class Transition> transition);
		void Shutdown();
		void UnloadData();

		void Draw();

		void AddSprite2D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture);
		void RemoveSprite2D(std::shared_ptr<class Sprite> sprite);

		void AddSprite3D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture);
		void RemoveSprite3D(std::shared_ptr<class Sprite> sprite);

		void ChangeBufferSprite(std::shared_ptr<class Sprite> sprite, TextureType type = TextureType::Image2D);

		void AddEffectComp(class EffectComponent *effect);
		void RemoveEffectComp(class EffectComponent *effect);

		std::vector<EffectComponent *> &GetEffectComponent() { return mEffectComp; }

		void AddVertexArray(const VertexArray &vArray, std::string_view name);

		void SetClearColor(const Vector3f &color)
		{
			if (color.x >= 0.f && color.y >= 0.f && color.z >= 0.f)
				clearColor = color;
		}

		Vector3f GetClearColor() { return this->clearColor; }

#ifdef EMSCRIPTEN
		es::ESRenderer &GetES()
		{
			return *(esRenderer.get());
		}
#endif
#ifndef EMSCRIPTEN
		vk::VKRenderer &GetVK()
		{
			return *(vkRenderer.get());
		}
		gl::GLRenderer &GetGL() { return *(glRenderer.get()); }
#endif

		class Texture *GetTexture(std::string_view fileName);
		class Texture *GetTextureFromMemory(const unsigned char *const buffer, const std::string &key);
		class Effect *GetEffect(const std::u16string &fileName);

		void SetViewMatrix(const Matrix4 &view) { mView = view; }
		Matrix4 GetViewMatrix() { return mView; }
		const Matrix4 &GetProjectionMatrix() { return mProjection; }

		SDL_Window *GetWindow()
		{
			if (!mWindow)
				return nullptr;
			return mWindow;
		}

	private:
		Vector3f clearColor = Color::Black;
		class Sprite2DComponent *transPic;

		std::shared_ptr<Transition> mTransition;
		// Map of textures loaded
		std::unordered_map<std::string, class Texture *> mTextures3D;
		// All the sprite components drawn
		std::vector<std::shared_ptr<class Sprite>> mSprite3Ds;
		std::vector<std::shared_ptr<class Sprite>> mSprite2Ds;

		std::unique_ptr<class Effect> mEffectManager;

		// All effects components drawn
		std::vector<class EffectComponent *> mEffectComp;

		// GameHandler
		std::shared_ptr<Scene> mScene;

		// Sprite vertex array
		class VertexArray *mSpriteVerts;

		// View/projection for 3D shaders
		Matrix4 mView;
		Matrix4 mProjection;

		// Window
		SDL_Window *mWindow;
		// Renderer
#ifndef EMSCRIPTEN
		std::unique_ptr<vk::VKRenderer> vkRenderer;
		std::unique_ptr<gl::GLRenderer> glRenderer;
#endif
#ifdef EMSCRIPTEN
		std::unique_ptr<es::ESRenderer> esRenderer;
#endif
		GraphicsAPI RendererAPI;
	};
}