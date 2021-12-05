#include "Vulkan/VKRenderer.h"
#include "OpenGL/GLRenderer.h"
#include "OpenGLES/ESRenderer.h"
#include <Nen.hpp>
#include "RendererHandle.hpp"

namespace nen
{
	std::shared_ptr<Renderer> RendererHandle::mRenderer = nullptr;
	Renderer::Renderer(GraphicsAPI api, std::shared_ptr<Window> window)
		: transPic(nullptr), mScene(nullptr), mWindow(window),
		  renderer(nullptr),
		  RendererAPI(api)
	{
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
		switch (RendererAPI)
		{
		case GraphicsAPI::Vulkan:
			renderer = std::make_unique<vk::VKRenderer>();
			break;
		case GraphicsAPI::OpenGL:
			renderer = std::make_unique<gl::GLRenderer>();
			break;
		default:
			break;
		}
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
		renderer = std::make_unique<es::ESRenderer>();
#endif
		renderer->SetRenderer(this);
		renderer->Initialize(mWindow);
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
	}

	void Renderer::Draw()
	{
		for (auto effect = mEffects.begin(); effect != mEffects.end();)
		{
			if ((*effect)->state == Effect::State::Dead)
				effect = mEffects.erase(effect);
			if (effect != mEffects.end())
				effect++;
		}
		renderer->Render();
	}

	void Renderer::AddDrawObject2D(std::shared_ptr<nen::DrawObject> drawObject, std::shared_ptr<Texture> texture)
	{
		renderer->AddDrawObject2D(drawObject, texture);
	}

	void Renderer::RemoveDrawObject2D(std::shared_ptr<DrawObject> drawObject)
	{
		renderer->RemoveDrawObject2D(drawObject);
	}

	void Renderer::AddDrawObject3D(std::shared_ptr<DrawObject> drawObject, std::shared_ptr<Texture> texture)
	{
		renderer->AddDrawObject3D(drawObject, texture);
	}

	void Renderer::RemoveDrawObject3D(std::shared_ptr<DrawObject> drawObject)
	{
		renderer->RemoveDrawObject3D(drawObject);
	}

	void Renderer::AddEffect(std::shared_ptr<Effect> effect)
	{
		renderer->LoadEffect(effect);
		mEffects.emplace_back(effect);
	}
	void Renderer::RemoveEffect(std::shared_ptr<Effect> effect)
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

	void Renderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
	{
		renderer->AddVertexArray(vArray, name);
	}

	void Renderer::UpdateVertexArray(const VertexArray &vArray, std::string_view name)
	{
		renderer->UpdateVertexArray(vArray, name);
	}

	void Renderer::AddModel(std::shared_ptr<Model> model, std::string_view name)
	{
		renderer->AddModel(model, name);
	}
	void Renderer::UpdateModel(std::shared_ptr<Model> model, std::string_view name)
	{
		renderer->UpdateModel(model, name);
	}

	void Renderer::LoadShader(const Shader &shaderInfo)
	{
		renderer->LoadShader(shaderInfo);
	}

	void Renderer::UnloadShader(const Shader &shaderInfo)
	{
		renderer->UnloadShader(shaderInfo);
	}

}
