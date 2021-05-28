#include <Engine.hpp>

#include <Engine/include/AudioSystem.h>
namespace nen
{
	AudioEvent::AudioEvent(std::shared_ptr<class AudioSystem> audiosystem, AudioType t, std::string_view name)
		: audiosys(audiosystem), mType(t), mName(name)
	{
		if (mType == AudioType::Music)
			m_param = &audiosys->musics[mName.data()];
		else
		{
			s_param = &audiosys->sounds[mName.data()];
		}
	}
	bool AudioEvent::IsValid()
	{
		if (mType == AudioType::Music)
			return audiosys && audiosys->musics.contains(mName.data());
		else
			return audiosys && audiosys->sounds.contains(mName.data());
	}

	void AudioEvent::Restart()
	{
		isPlaying = true;
		if (mType == AudioType::Music)
			;
		else
		{
			Stop();
			alSourcePlay(s_param->source_id);
		}
	}

	void AudioEvent::Stop(bool allowFadeOut /* true */)
	{
		isPlaying = false;
		if (mType == AudioType::Music)
			;
		else
		{
			alSourcePlay(s_param->source_id);
		}
	}

	void AudioEvent::SetPaused(bool pause)
	{
		isPaused = pause;
		if (mType == AudioType::Music)
			;
		else
		{
			alSourcePause(s_param->source_id);
		}
	}

	void AudioEvent::SetVolume(float value)
	{
		volume = value;
		if (mType == AudioType::Music)
			;
		else
		{
			alSourcef(s_param->source_id, AL_GAIN, value);
		}
	}

	void AudioEvent::SetPitch(float value)
	{
		pitch = value;
		if (mType == AudioType::Music)
			;
		else
		{
			alSourcef(s_param->source_id, AL_PITCH, value);
		}
	}

	void AudioEvent::SetPosition(Vector3f pos)
	{
		this->pos = pos;
		if (mType == AudioType::Music)
			;
		else
		{
			alSource3f(s_param->source_id, AL_POSITION, pos.x, pos.y, pos.z);
		}
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
