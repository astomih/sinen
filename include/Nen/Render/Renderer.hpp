#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Matrix4.hpp"
#include "../Window/Window.hpp"
#include "../Texture/TextureType.hpp"
#include "../Texture/Texture.hpp"
#include "../Vertex/VertexArray.hpp"
#include "../Color/Color.hpp"
#include <string>
#include <vector>
#include <unordered_map>

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

		void AddEffect(class Effect *effect);
		void RemoveEffect(class Effect *effect);

		std::vector<Effect *> &GetEffects() { return mEffects; }

		void AddVertexArray(const VertexArray &vArray, std::string_view name);

		void SetClearColor(const Color &color)
		{
			if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
				clearColor = color;
		}

		Color GetClearColor() { return this->clearColor; }

		class Texture *GetTexture(std::string_view fileName);
		class Texture *GetTextureFromMemory(const unsigned char *const buffer, const std::string &key);
		class EffectManager *GetEffect(const std::u16string &fileName);

		void SetViewMatrix(const Matrix4 &view) { mView = view; }
		Matrix4 GetViewMatrix() { return mView; }
		const Matrix4 &GetProjectionMatrix() { return mProjection; }

		struct SDL_Window *GetWindow()
		{
			if (!mWindow)
				return nullptr;
			return mWindow;
		}

	private:
		Color clearColor = Palette::Black;
		class Sprite2DComponent *transPic;

		std::shared_ptr<Transition> mTransition;
		// Map of textures loaded
		std::unordered_map<std::string, class Texture *> mTextures3D;
		// All the sprite components drawn
		std::vector<std::shared_ptr<class Sprite>> mSprite3Ds;
		std::vector<std::shared_ptr<class Sprite>> mSprite2Ds;

		// All effects components drawn
		std::vector<class Effect *> mEffects;

		// GameHandler
		std::shared_ptr<Scene> mScene;

		// Sprite vertex array
		class VertexArray *mSpriteVerts;

		// View/projection for 3D shaders
		Matrix4 mView;
		Matrix4 mProjection;

		// Window
		struct SDL_Window *mWindow;
		// Renderer
		std::unique_ptr<class IRenderer> renderer;
		GraphicsAPI RendererAPI;
	};

	class IRenderer
	{
	public:
		IRenderer() = default;
		virtual ~IRenderer() {}

		virtual void Initialize(struct SDL_Window *window) {}
		virtual void Shutdown() {}
		virtual void Render() {}
		virtual void AddVertexArray(const VertexArray &vArray, std::string_view name) {}
		virtual void ChangeBufferSprite(std::shared_ptr<class Sprite> sprite, const TextureType type) {}

		virtual void AddSprite2D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture) {}
		virtual void RemoveSprite2D(std::shared_ptr<class Sprite> sprite) {}

		virtual void AddSprite3D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture) {}
		virtual void RemoveSprite3D(std::shared_ptr<class Sprite> sprite) {}

		void SetRenderer(class Renderer *renderer) { mRenderer = renderer; }

	protected:
		struct SDL_Window *mWindow;
		class Renderer *mRenderer;
	};
}