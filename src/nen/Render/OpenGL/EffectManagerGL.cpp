#include "EffectManagerGL.h"
#include "../../effect/CustomTextureLoader.h"
#include "GLRenderer.h"
#include <nen.hpp>

namespace nen::gl
{
	EffectManagerGL::EffectManagerGL(GLRenderer* renderer)
		: mRenderer(renderer)
	{}
	void EffectManagerGL::Init()
	{
		// Create a renderer of effects
		renderer = ::EffekseerRendererGL::Renderer::Create(
			8000, EffekseerRendererGL::OpenGLDeviceType::OpenGL3);

		// Sprcify rendering modules
		manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
		manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
		manager->SetRingRenderer(renderer->CreateRingRenderer());
		manager->SetTrackRenderer(renderer->CreateTrackRenderer());
		manager->SetModelRenderer(renderer->CreateModelRenderer());

		// Specify a texture, model and material loader
		// It can be extended by yourself. It is loaded from a file on now.
		manager->SetTextureLoader(Effekseer::MakeRefPtr<CustomTextureLoader>(renderer->GetGraphicsDevice().Get()));
		manager->SetModelLoader(renderer->CreateModelLoader());
		manager->SetMaterialLoader(renderer->CreateMaterialLoader());

		// Specify a position of view
		auto g_position = ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(Math::ToRadians(90.f), Window::Size.x / Window::Size.y, 0.01f, 10000.f));

		// Specify a camera matrix
		renderer->SetCameraMatrix(
			::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, 0.0f, -1.f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));


	}
}