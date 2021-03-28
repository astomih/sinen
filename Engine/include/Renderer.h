#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include "Math.hpp"
#include "Window.hpp"
#include <VKRenderer.h>
#include <Engine/include/OpenGL/GLRenderer.h>

namespace nen
{

	enum class GraphicsAPI
	{
		OpenGL,
		Vulkan
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

		void AddSprite2D(class Sprite2DComponent *sprite);
		void RemoveSprite2D(class Sprite2DComponent *sprite);

		void AddSprite3D(class Sprite3DComponent *sprite);
		void RemoveSprite3D(class Sprite3DComponent *sprite);

		void AddEffectComp(class EffectComponent *effect);
		void RemoveEffectComp(class EffectComponent *effect);

		void AddVertexArray(const VertexArray &vArray, std::string_view name);

		vk::VKRenderer &GetVK() { return *(vkRenderer.get()); }
		gl::GLRenderer &GetGL() { return *(glRenderer.get()); }

		class Texture *GetTexture(std::string_view fileName);
		class Texture *GetTextureFromMemory(const unsigned char *const buffer, const std::string &key);
		class Effect *GetEffect(const std::u16string &fileName);

		void SetViewMatrix(const glm::mat4x4 &view) { mView = view; }
		glm::mat4x4 GetViewMatrix() { return mView; }
		const glm::mat4x4 &GetProjectionMatrix() { return mProjection; }

	private:
		class Sprite2DComponent *transPic;

		std::shared_ptr<Transition> mTransition;
		// Map of textures loaded
		std::unordered_map<std::string, class Texture *> mTextures3D;
		// Map of meshes loaded
		// Map of effects loaded
		std::unordered_map<std::string, class Effect *> mEffects;

		// All the sprite components drawn
		std::vector<class Sprite3DComponent *> mSprite3Ds;
		std::vector<class Sprite2DComponent *> mSprite2Ds;

		// All effects components drawn
		std::vector<class EffectComponent *> mEffectComp;

		// GameHandler
		std::shared_ptr<Scene> mScene;

		// Sprite vertex array
		class VertexArray *mSpriteVerts;

		// View/projection for 3D shaders
		glm::mat4x4 mView;
		glm::mat4x4 mProjection;

		// Window
		SDL_Window *mWindow;
		// Renderer
		std::unique_ptr<vk::VKRenderer> vkRenderer;
		std::unique_ptr<gl::GLRenderer> glRenderer;

		GraphicsAPI RendererAPI;
	};
}