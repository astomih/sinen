#include <Engine.hpp>

namespace nen
{
	SoundEvent::SoundEvent(class AudioSystem* system, unsigned int id)
		: mSystem(system), mID(id)
	{
	}

	SoundEvent::SoundEvent()
		: mSystem(nullptr), mID(0)
	{
	}

	bool SoundEvent::IsValid()
	{
		return true;
	}

	void SoundEvent::Restart()
	{
	}

	void SoundEvent::Stop(bool allowFadeOut /* true */)
	{
	}

	void SoundEvent::SetPaused(bool pause)
	{
	}

	void SoundEvent::SetVolume(float value)
	{
	}

	void SoundEvent::SetPitch(float value)
	{
	}

	void SoundEvent::SetParameter(const std::string& name, float value)
	{
	}

	bool SoundEvent::GetPaused() const
	{
		return true;
	}

	float SoundEvent::GetVolume() const
	{
		return 1.f;
	}

	float SoundEvent::GetPitch() const
	{
		return 0.f;
	}

	float SoundEvent::GetParameter(const std::string& name)
	{
		return 1.f;
	}
}
