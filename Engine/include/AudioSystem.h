#pragma once
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include "AudioEvent.h"
#include "Math.hpp"

namespace nen
{
	struct SoundParameters
	{
		Vector3f position;
		ALuint source_id;
		ALuint buffer_id;
	};
	struct MusicParameters
	{
	};
	enum class AudioType
	{
		Music,
		Sound
	};

	class AudioSystem : public std::enable_shared_from_this<AudioSystem>
	{
	public:
		//AudioSystem Constructor
		AudioSystem(std::shared_ptr<class Scene> scene);
		//AudioSystem Destructor
		~AudioSystem();

		bool Initialize();
		void Shutdown();
		void Update(float deltaTime);

		AudioEvent PlayEvent(std::string_view name, AudioType ae);

		void LoadAudioFile(std::string_view fileName, AudioType type);
		void UnloadAudioFile(std::string_view fileName);

		// For positional audio
		void SetListener(const Vector3f &pos, const Quaternion &quat);
		SoundParameters &GetSoundParameter(std::string_view name)
		{
#ifdef _DEBUG
			if (!sounds.contains(name.data()))
				std::cerr << "sound:" << name << "is not exist.";
#endif

			return sounds[name.data()];
		}

		MusicParameters &GetMusicParameter(std::string_view name)
		{
#ifdef _DEBUG
			if (!musics.contains(name.data()))
				std::cerr << "music:" << name << "is not exist.";
#endif

			return musics[name.data()];
		}

	protected:
		friend class AudioEvent;

	private:
		std::shared_ptr<Scene> mScene;
		std::unordered_map<std::string, SoundParameters> sounds;
		std::unordered_map<std::string, MusicParameters> musics;
		ALCdevice *device;
		ALCcontext *context;
	};
}