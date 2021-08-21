#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "VKBase.h"
#include "EffectManagerVK.h"
#include "VKRenderer.h"
#include <Nen.hpp>

namespace nen::vk
{
	EffectManagerVK::EffectManagerVK(VKRenderer *renderer)
		: mRenderer(renderer)
	{
	}
	void EffectManagerVK::Init()
	{
		// Create a renderer of effects
		::EffekseerRendererVulkan::RenderPassInformation renderPassInfo;
		renderPassInfo.DoesPresentToScreen = true;
		renderPassInfo.RenderTextureCount = 1;
		renderPassInfo.RenderTextureFormats[0] = VK_FORMAT_B8G8R8A8_UNORM;
		renderPassInfo.DepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		renderer = ::EffekseerRendererVulkan::Create(
			mRenderer->GetBase()->GetVkPhysicalDevice(),
			mRenderer->GetBase()->GetVkDevice(),
			mRenderer->GetBase()->GetVkQueue(),
			mRenderer->GetBase()->GetVkCommandPool(),
			mRenderer->GetBase()->GetSwapBufferCount(),
			renderPassInfo,
			8000);

		// Create a memory pool
		sfMemoryPool = ::EffekseerRenderer::CreateSingleFrameMemoryPool(renderer->GetGraphicsDevice());

		// Create a command list
		commandList = EffekseerRenderer::CreateCommandList(renderer->GetGraphicsDevice(), sfMemoryPool);

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
		auto g_position = ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovLH(Math::ToRadians(90.f), Window::Size.x / Window::Size.y, 0.01f, 10000.f));

		// Specify a camera matrix
		renderer->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtLH(g_position, ::Effekseer::Vector3D(0.0f, 0.0f, -1.f), ::Effekseer::Vector3D(0.0f, -1.0f, 0.0f)));
	}
}
#endif