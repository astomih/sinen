#pragma once
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include "AudioEvent.h"
#include "Math.hpp"

namespace nen
{
	class AudioSystem : public std::enable_shared_from_this<AudioSystem>
	{
	public:
		//AudioSystem Constructor
		AudioSystem(class Scene &scene);
		//AudioSystem Destructor
		~AudioSystem();

		bool Initialize();
		void Shutdown();
		void Update(float deltaTime);

		AudioEvent PlayEvent(std::string_view name, ALuint sourceID = 0);

		void LoadAudioFile(std::string_view fileName);
		void UnloadAudioFile(std::string_view fileName);

		ALuint NewSource(std::string_view name);
		void DeleteSource(ALuint sourceID);

		// For positional audio
		void SetListener(const Vector3f &pos, const Quaternion &direction);

	protected:
		friend class AudioEvent;

	private:
		Scene& mScene;
		std::unordered_map<std::string, ALuint> buffers;
		ALCdevice *device;
		ALCcontext *context;
	};
}