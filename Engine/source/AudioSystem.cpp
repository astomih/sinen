#include <Engine.hpp>
#include <SDL_log.h>
#include <vector>
#include <Scene.hpp>
namespace nen
{
	AudioSystem::AudioSystem(std::shared_ptr<Scene> scene)
		: mScene(scene)
	{
	}

	AudioSystem::~AudioSystem()
	{
	}

	bool AudioSystem::Initialize()
	{
		return true;
	}

	void AudioSystem::Shutdown()
	{
	}

	void AudioSystem::Update(float deltaTime)
	{
	}

	void AudioSystem::SetListener(const Matrix4& viewMatrix)
	{
		// Invert the view matrix to get the correct vectors
		Matrix4 invView = viewMatrix;
		invView.Invert();
		Vector3f v = invView.GetTranslation();
	}
}
