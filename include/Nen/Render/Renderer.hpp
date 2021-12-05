#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Matrix4.hpp"
#include "../Texture/TextureType.hpp"
#include "../Texture/Texture.hpp"
#include "../Vertex/VertexArray.hpp"
#include "../Color/Color.hpp"
#include "../Shader/Shader.hpp"
#include "../Model/Model.hpp"
#include <string>
#include <vector>
#include <memory>
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
		Renderer(GraphicsAPI api, std::shared_ptr<class Window> window);
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

		void AddDrawObject2D(std::shared_ptr<class DrawObject> drawObject, std::shared_ptr<Texture> texture);
		void RemoveDrawObject2D(std::shared_ptr<class DrawObject> drawObject);

		void AddDrawObject3D(std::shared_ptr<class DrawObject> drawObject, std::shared_ptr<Texture> texture);
		void RemoveDrawObject3D(std::shared_ptr<class DrawObject> drawObject);

		void UpdateVertexArray(std::shared_ptr<class DrawObject> drawObject, TextureType type = TextureType::Image2D);

		void AddEffect(std::shared_ptr<class Effect> effect);
		void RemoveEffect(std::shared_ptr<class Effect> effect);

		void AddGUI(std::shared_ptr<class UIScreen> ui);
		void RemoveGUI(std::shared_ptr<class UIScreen> ui);

		std::vector<std::shared_ptr<class Effect>> &GetEffects() { return mEffects; }

		void AddVertexArray(const VertexArray &vArray, std::string_view name);
		void UpdateVertexArray(const VertexArray &vArray, std::string_view name);

		void AddModel(std::shared_ptr<class Model>, std::string_view name);
		void UpdateModel(std::shared_ptr<class Model>, std::string_view name);

		void SetClearColor(const Color &color)
		{
			if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
				clearColor = color;
		}

		Color GetClearColor() { return this->clearColor; }

		std::shared_ptr<class EffectManager> GetEffect(const std::u16string &fileName);

		void SetViewMatrix(const Matrix4 &view) { mView = view; }
		const Matrix4 &GetViewMatrix() { return mView; }
		void SetProjectionMatrix(const Matrix4 &projection) { mProjection = projection; }
		const Matrix4 &GetProjectionMatrix() { return mProjection; }

		std::shared_ptr<class Window> GetWindow() { return mWindow; }

		void toggleShowImGui() { showImGui = !showImGui; }
		bool isShowImGui() { return showImGui; }

		void LoadShader(const Shader &shaderinfo);
		void UnloadShader(const Shader &shaderinfo);

	private:
		Color clearColor = Palette::Black;
		class Draw2DComponent *transPic;

		std::vector<std::shared_ptr<class Effect>> mEffects;

		// GameHandler
		std::shared_ptr<Scene> mScene;

		// Sprite vertex array
		class VertexArray *mSpriteVerts;

		// View/projection for 3D shaders
		Matrix4 mView;
		Matrix4 mProjection;

		// Window
		std::shared_ptr<class Window> mWindow;
		// Renderer
		std::unique_ptr<class IRenderer> renderer;
		GraphicsAPI RendererAPI;
		bool showImGui = false;
	};

	class IRenderer
	{
	public:
		IRenderer() = default;
		virtual ~IRenderer() {}

		virtual void Initialize(std::shared_ptr<Window> window) {}
		virtual void Shutdown() {}
		virtual void Render() {}
		virtual void AddVertexArray(const VertexArray &vArray, std::string_view name) {}
		virtual void UpdateVertexArray(const VertexArray &vArray, std::string_view name) {}

		virtual void AddModel(std::shared_ptr<class Model> model, std::string_view name) {}
		virtual void UpdateModel(std::shared_ptr<class Model> model, std::string_view name) {}

		virtual void AddDrawObject2D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture) {}
		virtual void RemoveDrawObject2D(std::shared_ptr<class DrawObject> sprite) {}

		virtual void AddDrawObject3D(std::shared_ptr<class DrawObject> drawObject, std::shared_ptr<Texture> texture) {}
		virtual void RemoveDrawObject3D(std::shared_ptr<class DrawObject> drawObject) {}

		virtual void AddGUI(std::shared_ptr<class UIScreen> ui) {}
		virtual void RemoveGUI(std::shared_ptr<class UIScreen> ui) {}

		virtual void LoadEffect(std::shared_ptr<Effect> effect) {}

		virtual void SetRenderer(class Renderer *renderer) {}

		virtual void LoadShader(const Shader &shaderInfo) {}
		virtual void UnloadShader(const Shader &shaderInfo) {}
	};
}