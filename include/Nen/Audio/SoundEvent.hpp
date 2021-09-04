#pragma once
#include "../Math/Math.hpp"
#include "../Math/Vector3.hpp"
#include <string>
#include <cstdint>
namespace nen
{
	struct SoundParameter
	{
		Vector3 position;
		uint32_t source_id;
		uint32_t buffer_id;
	};

	class SoundEvent
	{
	public:
		SoundEvent() = default;
		bool IsValid();
		// Restart event from begining
		void Restart();
		// Stop this event
		void Stop(bool allowFadeOut = true);
		// Setters
		void SetPaused(bool pause);
		void SetVolume(float value);
		void SetPitch(float value);
		void SetPosition(Vector3 pos);
		// Getters
		bool GetPaused();
		float GetVolume();
		float GetPitch();
		std::string GetName();
		const Vector3 &GetPosition();

	protected:
		friend class SoundSystem;
		SoundEvent(std::shared_ptr<class SoundSystem> audiosystem, std::string_view name, uint32_t sourceID = 0);

	private:
		std::shared_ptr<class SoundSystem> audiosys;
		std::string mName;
		SoundParameter param;
		float volume = 1.f;
		float pitch = 1.f;
		Vector3 pos;
		bool isPlaying = true;
		bool isPaused = false;
	};
}