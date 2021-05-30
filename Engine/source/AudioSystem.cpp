#include <Engine.hpp>
#include <SDL_log.h>
#include <vector>
#include <Scene.hpp>
namespace nen
{
	Vector3f Calc(const Quaternion &r);
	namespace detail
	{
		static LPALCTRACEDEVICELABEL palcTraceDeviceLabel;
		static LPALCTRACECONTEXTLABEL palcTraceContextLabel;
		static LPALTRACEPUSHSCOPE palTracePushScope;
		static LPALTRACEPOPSCOPE palTracePopScope;
		static LPALTRACEMESSAGE palTraceMessage;
		static LPALTRACEBUFFERLABEL palTraceBufferLabel;
		static LPALTRACESOURCELABEL palTraceSourceLabel;
		static int check_openal_error(const char *where)
		{
			const ALenum err = alGetError();
			if (err != AL_NONE)
			{
				printf("OpenAL Error at %s! %s (%u)\n", where, alGetString(err), (unsigned int)err);
				return 1;
			}
			return 0;
		}
		static ALenum get_openal_format(const SDL_AudioSpec *spec)
		{
			if ((spec->channels == 1) && (spec->format == AUDIO_U8))
			{
				return AL_FORMAT_MONO8;
			}
			else if ((spec->channels == 1) && (spec->format == AUDIO_S16SYS))
			{
				return AL_FORMAT_MONO16;
			}
			else if ((spec->channels == 2) && (spec->format == AUDIO_U8))
			{
				return AL_FORMAT_STEREO8;
			}
			else if ((spec->channels == 2) && (spec->format == AUDIO_S16SYS))
			{
				return AL_FORMAT_STEREO16;
			}
			else if ((spec->channels == 1) && (spec->format == AUDIO_F32SYS))
			{
				return alIsExtensionPresent("AL_EXT_FLOAT32") ? alGetEnumValue("AL_FORMAT_MONO_FLOAT32") : AL_NONE;
			}
			else if ((spec->channels == 2) && (spec->format == AUDIO_F32SYS))
			{
				return alIsExtensionPresent("AL_EXT_FLOAT32") ? alGetEnumValue("AL_FORMAT_STEREO_FLOAT32") : AL_NONE;
			}
			return AL_NONE;
		}
	}
	AudioSystem::AudioSystem(std::shared_ptr<Scene> scene)
		: mScene(scene)
	{
	}

	AudioSystem::~AudioSystem()
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	bool AudioSystem::Initialize()
	{
		device = alcOpenDevice(NULL);
		if (!device)
		{
			printf("Couldn't open OpenAL default device.\n");
			return -1;
		}
		if (alcIsExtensionPresent(device, "ALC_EXT_trace_info"))
		{
			detail::palcTraceDeviceLabel = (LPALCTRACEDEVICELABEL)alcGetProcAddress(device, "alcTraceDeviceLabel");
			detail::palcTraceContextLabel = (LPALCTRACECONTEXTLABEL)alcGetProcAddress(device, "alcTraceContextLabel");
		}
		context = alcCreateContext(device, NULL);
		if (!context)
		{
			printf("Couldn't create OpenAL context.\n");
			alcCloseDevice(device);
			return -1;
		}

		if (detail::palcTraceDeviceLabel)
			detail::palcTraceDeviceLabel(device, "The playback device");
		if (detail::palcTraceContextLabel)
			detail::palcTraceContextLabel(context, "Main context");

		alcMakeContextCurrent(context);

		if (alIsExtensionPresent("AL_EXT_trace_info"))
		{
			detail::palTracePushScope = (LPALTRACEPUSHSCOPE)alGetProcAddress("alTracePushScope");
			detail::palTracePopScope = (LPALTRACEPOPSCOPE)alGetProcAddress("alTracePopScope");
			detail::palTraceMessage = (LPALTRACEMESSAGE)alGetProcAddress("alTraceMessage");
			detail::palTraceBufferLabel = (LPALTRACEBUFFERLABEL)alGetProcAddress("alTraceBufferLabel");
			detail::palTraceSourceLabel = (LPALTRACESOURCELABEL)alGetProcAddress("alTraceSourceLabel");
		}
		return true;
	}

	void AudioSystem::Shutdown()
	{
	}

	void AudioSystem::Update(float deltaTime)
	{
	}

	AudioEvent AudioSystem::PlayEvent(std::string_view name, AudioType ae)
	{
		AudioEvent e(shared_from_this(), ae, name);
		return e;
	}

	void AudioSystem::SetListener(const Vector3f &pos, const Quaternion &quat)
	{
		alListener3f(AL_POSITION, pos.x, pos.z, pos.y);
		Vector3f d = Vector3f::Transform(Vector3f::UnitX, quat);
		float d2[6] = {d.z, -d.x, d.y, 1, 1, 0};
		alListenerfv(AL_ORIENTATION, d2);
	}

	void AudioSystem::LoadAudioFile(std::string_view fileName, AudioType type)
	{
		if (type == AudioType::Music)
		{
		}
		else
		{
			SDL_AudioSpec spec;
			ALenum alfmt = AL_NONE;
			Uint8 *buf = NULL;
			Uint32 buflen = 0;
			ALuint sid = 0;
			ALuint bid = 0;

			if (!SDL_LoadWAV(fileName.data(), &spec, &buf, &buflen))
			{
				printf("Loading '%s' failed! %s\n", fileName.data(), SDL_GetError());
				return;
			}
			else if ((alfmt = detail::get_openal_format(&spec)) == AL_NONE)
			{
				printf("Can't queue '%s', format not supported by the AL.\n", fileName);
				SDL_FreeWAV(buf);
				return;
			}

			detail::check_openal_error("startup");

			printf("Now queueing '%s'...\n", fileName);

			if (detail::palTracePushScope)
				detail::palTracePushScope("Initial setup");

			alGenSources(1, &sid);
			if (detail::check_openal_error("alGenSources"))
			{
				SDL_FreeWAV(buf);
				return;
			}

			if (detail::palTraceSourceLabel)
				detail::palTraceSourceLabel(sid, "Moving source");

			alGenBuffers(1, &bid);
			if (detail::check_openal_error("alGenBuffers"))
			{
				alDeleteSources(1, &sid);
				detail::check_openal_error("alDeleteSources");
				SDL_FreeWAV(buf);
				return;
			}

			if (detail::palTraceBufferLabel)
				detail::palTraceBufferLabel(bid, "Sound effect");

			alBufferData(bid, alfmt, buf, buflen, spec.freq);
			SDL_FreeWAV(buf);
			detail::check_openal_error("alBufferData");

			SoundParameters param;
			param.buffer_id = bid;
			param.source_id = sid;
			sounds.emplace(fileName.data(), param);

			alSourcei(sid, AL_BUFFER, bid);
			detail::check_openal_error("alSourcei");
			alSourcei(sid, AL_LOOPING, AL_TRUE);
			detail::check_openal_error("alSourcei");
		}
	}

	void AudioSystem::UnloadAudioFile(std::string_view fileName)
	{
		std::string name = fileName.data();
		if (sounds.contains(name))
		{
			alDeleteSources(1, &sounds[name].source_id);
			detail::check_openal_error("alDeleteSources");
			alDeleteBuffers(1, &sounds[name].buffer_id);
			detail::check_openal_error("alDeleteBuffers");
			sounds.erase(name);
		}
	}
	Vector3f Calc(const Quaternion &r)
	{
		float x = r.x;
		float y = r.y;
		float z = r.z;
		float w = r.w;

		float x2 = x * x;
		float y2 = y * y;
		float z2 = z * z;

		float xy = x * y;
		float xz = x * z;
		float yz = y * z;
		float wx = w * x;
		float wy = w * y;
		float wz = w * z;

		// 1 - 2y^2 - 2z^2
		float m00 = 1.f - (2.f * y2) - (2.f * z2);

		// 2xy + 2wz
		float m01 = (2.f * xy) + (2.f * wz);

		// 2xy - 2wz
		float m10 = (2.f * xy) - (2.f * wz);

		// 1 - 2x^2 - 2z^2
		float m11 = 1.f - (2.f * x2) - (2.f * z2);

		// 2xz + 2wy
		float m20 = (2.f * xz) + (2.f * wy);

		// 2yz+2wx
		float m21 = (2.f * yz) - (2.f * wx);

		// 1 - 2x^2 - 2y^2
		float m22 = 1.f - (2.f * x2) - (2.f * y2);

		float tx, ty, tz;

		if (m21 >= 0.99 && m21 <= 1.01)
		{
			tx = Math::Pi / 2.f;
			ty = 0;
			tz = Math::Atan2(m10, m00);
		}
		else if (m21 >= -1.01f && m21 <= -0.99f)
		{
			tx = -Math::Pi / 2.f;
			ty = 0;
			tz = Math::Atan2(m10, m00);
		}
		else
		{
			tx = std::asin(-m21);
			ty = Math::Atan2(m20, m22);
			tz = Math::Atan2(m01, m11);
		}

		return Vector3f(Math::ToDegrees(tx), Math::ToDegrees(ty), Math::ToDegrees(tz));
	}
}