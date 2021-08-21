#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Quaternion.hpp"
#include "AudioEvent.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <unordered_map>
#include <string>

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
		void SetListener(const Vector3 &pos, const Quaternion &direction);

	protected:
		friend class AudioEvent;

	private:
		Scene &mScene;
		std::unordered_map<std::string, ALuint> buffers;
		ALCdevice *device;
		ALCcontext *context;
	};
}