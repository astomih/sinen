#include "EffectManagerGL.h"
#include "../../Effect/CustomTextureLoader.h"
#include "GLRenderer.h"
#include <Nen.hpp>

namespace nen::gl
{
	EffectManagerGL::EffectManagerGL(GLRenderer *renderer)
		: mRenderer(renderer)
	{
	}
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
		auto g_position = ::Effekseer::Vector3D(0.0f, 30.f, 5.f);

		// Specify a projection matrix
		renderer->SetProjectionMatrix(
			::Effekseer::Matrix44().PerspectiveFovRH(Math::ToRadians(70.f), mRenderer->GetWindow()->Size().x / mRenderer->GetWindow()->Size().y, 0.1f, 1000.f));

		auto m2 = ::Effekseer::Matrix44().LookAtRH(g_position, ::Effekseer::Vector3D(0.0f, -5.f, -10.f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f));
		// Specify a camera matrix
		renderer->SetCameraMatrix(m2);
	}
}