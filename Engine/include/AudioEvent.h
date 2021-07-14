#pragma once
#include <Engine/include/AudioSystem.h>
#include <string>
#include "Math.hpp"
namespace nen
{
	struct AudioParameter
	{
		Vector3f position;
		ALuint source_id;
		ALuint buffer_id;
	};

	class AudioEvent
	{
	public:
		AudioEvent() = default;
		bool IsValid();
		// Restart event from begining
		void Restart();
		// Stop this event
		void Stop(bool allowFadeOut = true);
		// Setters
		void SetPaused(bool pause);
		void SetVolume(float value);
		void SetPitch(float value);
		void SetPosition(Vector3f pos);
		// Getters
		bool GetPaused();
		float GetVolume();
		float GetPitch();
		std::string GetName();
		const Vector3f &GetPosition();

	protected:
		friend class AudioSystem;
		AudioEvent(std::shared_ptr<class AudioSystem> audiosystem, std::string_view name, ALuint sourceID = 0);

	private:
		std::shared_ptr<AudioSystem> audiosys;
		std::string mName;
		AudioParameter param;
		float volume = 1.f;
		float pitch = 1.f;
		Vector3f pos;
		bool isPlaying = true;
		bool isPaused = false;
	};
}