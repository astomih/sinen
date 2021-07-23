#include <Engine.hpp>
#include <Effekseer.h>
#ifndef EMSCRIPTEN
#include <EffekseerRendererVulkan.h>
#endif
#ifdef EMSCRIPTEN
#include <Renderer.h>
#include <EffekseerRenderer/EffekseerRendererGL.MaterialLoader.h>
#include <EffekseerRenderer/EffekseerRendererGL.RendererImplemented.h>
#include <EffekseerRenderer/GraphicsDevice.h>
#include <SDL_image.h>
static void ArrayToMatrix44(const float *array, Effekseer::Matrix44 &matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			matrix.Values[i][j] = array[i * 4 + j];
		}
	}
}

static void ArrayToMatrix43(const float *array, Effekseer::Matrix43 &matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			matrix.Value[i][j] = array[i * 4 + j];
		}
	}
}

static bool isEffekseerLogEnabled = false;

static void PrintEffekseerLog(const std::string &message)
{
	if (isEffekseerLogEnabled)
	{
		printf("%s\n", message.c_str());
	}
}
using namespace Effekseer;

class CustomTextureLoader : public TextureLoader
{
private:
	::Effekseer::Backend::GraphicsDevice *graphicsDevice_ = nullptr;

public:
	CustomTextureLoader(::Effekseer::Backend::GraphicsDevice *graphicsDevice) : graphicsDevice_(graphicsDevice) {}

	~CustomTextureLoader() = default;

public:
	Effekseer::TextureRef Load(const EFK_CHAR *path, Effekseer::TextureType textureType) override
	{


		std::array<char, 260> path8;
		Effekseer::ConvertUtf16ToUtf8(path8.data(), static_cast<int32_t>(path8.size()), path);
		SDL_Surface* surf = ::IMG_Load(path8.data());
		::SDL_LockSurface(surf);
		auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
		formatbuf->BytesPerPixel = 4;
		auto imagedata = ::SDL_ConvertSurface(surf, formatbuf, 0);
		SDL_UnlockSurface(surf);
		// Generate a GL texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, imagedata->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Use linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		// Load texture from image

		std::string pathStr = path8.data();

		auto backend = static_cast<EffekseerRendererGL::Backend::GraphicsDevice *>(graphicsDevice_)
						   ->CreateTexture(texture, true, [texture, pathStr]() -> void
										   {
											   glDeleteTextures(1, &texture);
											   PrintEffekseerLog("Effekseer : Unload : " + pathStr);
										   });
		auto textureData = Effekseer::MakeRefPtr<Effekseer::Texture>();
		textureData->SetBackend(backend);

		PrintEffekseerLog("Effekseer : Load : " + pathStr);

		SDL_FreeSurface(surf);

		return textureData;
	}

	void Unload(Effekseer::TextureRef data) override {}
};
#endif

#ifndef EMSCRIPTEN
namespace nen::vk
{
	EffectVK::EffectVK(Effekseer::RefPtr<EffekseerRenderer::SingleFrameMemoryPool> SFMemoryPool,
					   Effekseer::RefPtr<EffekseerRenderer::CommandList> commandlist,
					   EffekseerRenderer::RendererRef rendererRef,
					   Effekseer::ManagerRef managerRef,
					   Effect &effect)
		: sfMemoryPool(SFMemoryPool), commandList(commandlist), renderer(rendererRef), manager(managerRef), owner(effect)
	{
	}

	Effekseer::EffectRef EffectVK::GetEffect(const std::u16string &name)
	{
		return this->owner.GetEffectRef(name);
	}

}

namespace nen::gl
{
	EffectGL::EffectGL(
		EffekseerRenderer::RendererRef rendererRef,
		Effekseer::ManagerRef managerRef,
		Effect &effect)
		: renderer(rendererRef), manager(managerRef), owner(effect)
	{
	}

	::Effekseer::EffectRef EffectGL::GetEffect(const std::u16string &name)
	{
		return this->owner.GetEffectRef(name);
	}

}
#endif
namespace nen::es
{
	EffectGL::EffectGL(
		EffekseerRenderer::RendererRef rendererRef,
		Effekseer::ManagerRef managerRef,
		Effect &effect)
		: renderer(rendererRef), manager(managerRef), owner(effect)
	{
	}

	Effekseer::EffectRef EffectGL::GetEffect(const std::u16string &name)
	{
		return this->owner.GetEffectRef(name);
	}

}
namespace nen
{
	Effect::Effect()
#ifndef EMSCRIPTEN
		: manager(Effekseer::Manager::Create(8000))
#else
		: manager(Effekseer::Manager::Create(1000))
#endif
	{
	}
	Effekseer::EffectRef Effect::GetEffectRef(const std::u16string &filePath)
	{

		if (effects.find(filePath) != effects.end())
		{
			//Load an effect
			auto effect = ::Effekseer::Effect::Create(this->manager, filePath.c_str(), 10.f);
			effects.emplace(filePath, effect);
			return effect;
		}
		else
		{
			return effects[filePath];
		}
	}

#ifndef EMSCRIPTEN
	void Effect::Init(class nen::vk::VKRenderer *vkrenderer, class nen::vk::VKBase *vkbase)
	{
		// Create a renderer of effects
		::EffekseerRendererVulkan::RenderPassInformation renderPassInfo;
		renderPassInfo.DoesPresentToScreen = true;
		renderPassInfo.RenderTextureCount = 1;
		renderPassInfo.RenderTextureFormats[0] = VK_FORMAT_B8G8R8A8_UNORM;
		renderPassInfo.DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		auto renderer = ::EffekseerRendererVulkan::Create(
			vkbase->GetVkPhysicalDevice(),
			vkbase->GetVkDevice(),
			vkbase->GetVkQueue(),
			vkbase->GetVkCommandPool(),
			vkbase->GetSwapBufferCount(),
			renderPassInfo,
			8000);

		// Create a memory pool
		auto sfMemoryPoolEfk = ::EffekseerRenderer::CreateSingleFrameMemoryPool(renderer->GetGraphicsDevice());

		// Create a command list
		auto commandListEfk = EffekseerRenderer::CreateCommandList(renderer->GetGraphicsDevice(), sfMemoryPoolEfk);

		// Sprcify rendering modules
		manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
		manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
		manager->SetRingRenderer(renderer->CreateRingRenderer());
		manager->SetTrackRenderer(renderer->CreateTrackRenderer());
		manager->SetModelRenderer(renderer->CreateModelRenderer());

		// Specify a texture, model and material loader
		// It can be extended by yourself. It is loaded from a file on now.
		manager->SetTextureLoader(renderer->CreateTextureLoader());
		manager->SetModelLoader(renderer->CreateModelLoader());
		manager->SetMaterialLoader(renderer->CreateMaterialLoader());

		// Specify a position of view
		auto g_position = ::Effekseer::Vector3D(10.0f, 5.0f, 20.0f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(90.0f / 180.0f * 3.14f, Window::Size.x / Window::Size.y, 1.0f, 500.0f));

		// Specify a camera matrix
		renderer->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

		vkrenderer->SetEffect(std::make_unique<vk::EffectVK>(sfMemoryPoolEfk, commandListEfk, renderer, manager, *this));
	}

	void Effect::Init(nen::gl::GLRenderer *glrenderer)
	{
		// Create a renderer of effects
		auto renderer = ::EffekseerRendererGL::Renderer::Create(
			8000, EffekseerRendererGL::OpenGLDeviceType::OpenGL3);

		// Sprcify rendering modules
		manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
		manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
		manager->SetRingRenderer(renderer->CreateRingRenderer());
		manager->SetTrackRenderer(renderer->CreateTrackRenderer());
		manager->SetModelRenderer(renderer->CreateModelRenderer());

		// Specify a texture, model and material loader
		// It can be extended by yourself. It is loaded from a file on now.
		manager->SetTextureLoader(renderer->CreateTextureLoader());
		manager->SetModelLoader(renderer->CreateModelLoader());
		manager->SetMaterialLoader(renderer->CreateMaterialLoader());

		// Specify a position of view
		auto g_position = ::Effekseer::Vector3D(10.0f, 5.0f, 20.0f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(90.0f / 180.0f * 3.14f, Window::Size.x / Window::Size.y, 1.0f, 500.0f));

		// Specify a camera matrix
		renderer->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

		glrenderer->SetEffect(std::make_unique<gl::EffectGL>(renderer, manager, *this));
	}
#endif
#ifdef EMSCRIPTEN
	void Effect::Init(::nen::es::ESRenderer *glrenderer)
	{
		// Create a renderer of effects
		auto renderer = ::EffekseerRendererGL::Renderer::Create(
			1000,
			EffekseerRendererGL::OpenGLDeviceType::OpenGLES3);

		// Sprcify rendering modules
		manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
		manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
		manager->SetRingRenderer(renderer->CreateRingRenderer());
		manager->SetTrackRenderer(renderer->CreateTrackRenderer());
		manager->SetModelRenderer(renderer->CreateModelRenderer());

		// Specify a texture, model and material loader
		manager->SetTextureLoader(Effekseer::MakeRefPtr<CustomTextureLoader>(renderer->GetGraphicsDevice().Get()));
		manager->SetModelLoader(renderer->CreateModelLoader(&fileInterface));
		manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>(&fileInterface));

		manager->SetMaterialLoader(Effekseer::MakeRefPtr<EffekseerRendererGL::MaterialLoader>(
			renderer->GetGraphicsDevice().DownCast<EffekseerRendererGL::Backend::GraphicsDevice>(), &fileInterface, false));

		// Specify a position of view
		auto g_position = ::Effekseer::Vector3D(10.0f, 5.0f, 20.0f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(90.0f / 180.0f * 3.14f, Window::Size.x / Window::Size.y, 1.0f, 500.0f));

		// Specify a camera matrix
		renderer->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

		glrenderer->SetEffect(std::make_unique<es::EffectGL>(renderer, manager, *this));
	}
#endif
	void Effect::Draw()
	{
	}

	void Effect::Remove()
	{
	}
}

#ifdef EMSCRIPTEN
#endif
