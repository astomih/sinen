#pragma once
#include <unordered_map>
#include <string>
#include "SoundEvent.h"
#include "Math.hpp"

class AudioSystem
{
public:
	//AudioSystem Constructor
	AudioSystem(std::shared_ptr<class Scene> scene);
	//AudioSystem Destructor
	~AudioSystem();

	/// <summary>
	/// Initialize FMOD Studio API
	/// </summary>
	/// <returns> Success or not success</returns>
	bool Initialize();
	/// <summary>
	/// Shutdown
	/// </summary>
	void Shutdown();

	void Update(float deltaTime);

	// For positional audio
	void SetListener(const Matrix4 &viewMatrix);

protected:
	friend class SoundEvent;

private:
	std::shared_ptr<Scene> mScene;
};