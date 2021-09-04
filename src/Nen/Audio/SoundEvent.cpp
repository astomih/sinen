#include <AL/al.h>
#include <AL/alc.h>
#include <Nen.hpp>
namespace nen
{
	SoundEvent::SoundEvent(std::shared_ptr<SoundSystem> audiosystem, std::string_view name, uint32_t sourceID)
		: audiosys(audiosystem), mName(name)
	{
		ALint buf;
		alGetSourcei(sourceID, AL_BUFFER, &buf);
		alSourcePlay(sourceID);
		param.source_id = sourceID;
		param.buffer_id = buf;
	}
	bool SoundEvent::IsValid()
	{
		return audiosys && audiosys->buffers.contains(mName.data());
	}

	void SoundEvent::Restart()
	{
		isPlaying = true;
		Stop();
		alSourcePlay(param.source_id);
	}

	void SoundEvent::Stop(bool allowFadeOut /* true */)
	{
		isPlaying = false;
		alSourceStop(param.source_id);
	}

	void SoundEvent::SetPaused(bool pause)
	{
		isPaused = pause;
		alSourcePause(param.source_id);
	}

	void SoundEvent::SetVolume(float value)
	{
		volume = value;
		alSourcef(param.source_id, AL_GAIN, value);
	}

	void SoundEvent::SetPitch(float value)
	{
		pitch = value;
		alSourcef(param.source_id, AL_PITCH, value);
	}

	void SoundEvent::SetPosition(Vector3 pos)
	{
		this->pos = pos;
		alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
	}

	bool SoundEvent::GetPaused()
	{
		return isPaused;
	}

	float SoundEvent::GetVolume()
	{
		return volume;
	}

	float SoundEvent::GetPitch()
	{
		return pitch;
	}

	const Vector3 &SoundEvent::GetPosition()
	{
		return pos;
	}

	std::string SoundEvent::GetName()
	{
		return mName;
	}

}
