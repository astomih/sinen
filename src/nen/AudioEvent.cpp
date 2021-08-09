#include <nen.hpp>
namespace nen
{
	AudioEvent::AudioEvent(std::shared_ptr<class AudioSystem> audiosystem, std::string_view name, ALuint sourceID)
		: audiosys(audiosystem), mName(name)
	{
		ALint buf;
		alGetSourcei(sourceID, AL_BUFFER, &buf);
		alSourcef(sourceID, AL_VELOCITY, 25.f);
		alSourcePlay(sourceID);
		param.source_id = sourceID;
		param.buffer_id = buf;
	}
	bool AudioEvent::IsValid()
	{
		return audiosys && audiosys->buffers.contains(mName.data());
	}

	void AudioEvent::Restart()
	{
		isPlaying = true;
		Stop();
		alSourcePlay(param.source_id);
	}

	void AudioEvent::Stop(bool allowFadeOut /* true */)
	{
		isPlaying = false;
		alSourceStop(param.source_id);
	}

	void AudioEvent::SetPaused(bool pause)
	{
		isPaused = pause;
		alSourcePause(param.source_id);
	}

	void AudioEvent::SetVolume(float value)
	{
		volume = value;
		alSourcef(param.source_id, AL_GAIN, value);
	}

	void AudioEvent::SetPitch(float value)
	{
		pitch = value;
		alSourcef(param.source_id, AL_PITCH, value);
	}

	void AudioEvent::SetPosition(Vector3f pos)
	{
		this->pos = pos;
		alSource3f(param.source_id, AL_POSITION, pos.x, pos.y, pos.z);
	}

	bool AudioEvent::GetPaused()
	{
		return isPaused;
	}

	float AudioEvent::GetVolume()
	{
		return volume;
	}

	float AudioEvent::GetPitch()
	{
		return pitch;
	}

	const Vector3f &AudioEvent::GetPosition()
	{
		return pos;
	}

	std::string AudioEvent::GetName()
	{
		return mName;
	}

}
