#include <AL/al.h>
#include <AL/alc.h>
#include <Nen.hpp>
#include <SDL_log.h>
#include <SDL_mixer.h>
#include <vector>

namespace nen {
vector3 calculate(const quaternion &r);
namespace detail {
static int check_openal_error(const char *where) {
  const ALenum err = alGetError();
  if (err != AL_NONE) {
    printf("OpenAL Error at %s! %s (%u)\n", where, alGetString(err),
           (unsigned int)err);
    return 1;
  }
  return 0;
}
static ALenum get_openal_format(const SDL_AudioSpec *spec) {
  if ((spec->channels == 1) && (spec->format == AUDIO_U8)) {
    return AL_FORMAT_MONO8;
  } else if ((spec->channels == 1) && (spec->format == AUDIO_S16SYS)) {
    return AL_FORMAT_MONO16;
  } else if ((spec->channels == 2) && (spec->format == AUDIO_U8)) {
    return AL_FORMAT_STEREO8;
  } else if ((spec->channels == 2) && (spec->format == AUDIO_S16SYS)) {
    return AL_FORMAT_STEREO16;
  } else if ((spec->channels == 1) && (spec->format == AUDIO_F32SYS)) {
    return alIsExtensionPresent("AL_EXT_FLOAT32")
               ? alGetEnumValue("AL_FORMAT_MONO_FLOAT32")
               : AL_NONE;
  } else if ((spec->channels == 2) && (spec->format == AUDIO_F32SYS)) {
    return alIsExtensionPresent("AL_EXT_FLOAT32")
               ? alGetEnumValue("AL_FORMAT_STEREO_FLOAT32")
               : AL_NONE;
  }
  return AL_NONE;
}
} // namespace detail
sound_system::sound_system() {}

sound_system::~sound_system() {
  for (auto &i : buffers) {
    alDeleteBuffers(1, &i.second);
  }
  alcMakeContextCurrent(NULL);
  alcDestroyContext((ALCcontext *)context);
  alcCloseDevice((ALCdevice *)device);
}

bool sound_system::Initialize() {
  device = (void *)alcOpenDevice(NULL);
  if (!device) {
    printf("Couldn't open OpenAL default device.\n");
    return -1;
  }
  context = (void *)alcCreateContext((ALCdevice *)device, NULL);
  if (!context) {
    printf("Couldn't create OpenAL context.\n");
    alcCloseDevice((ALCdevice *)device);
    return -1;
  }

  if (!alcMakeContextCurrent((ALCcontext *)context)) {
    printf("Couldn't create OpenAL context.\n");
    alcMakeContextCurrent(NULL);
    alcCloseDevice((ALCdevice *)device);
  }
  return true;
}

void sound_system::Shutdown() {}

void sound_system::Update(float deltaTime) {}

void sound_system::SetListener(const vector3 &pos,
                               const quaternion &direction) {
  alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
  auto at = vector3::Transform(vector3::NegUnitZ, direction);
  auto up = vector3::Transform(vector3::UnitY, direction);
  float ori[6] = {at.x, at.y, at.z, up.x, up.y, up.z};
  alListenerfv(AL_ORIENTATION, ori);
}

void sound_system::LoadAudioFile(std::string_view fileName) {
  if (buffers.contains(fileName.data())) {
    return;
  }
  SDL_AudioSpec spec;
  ALenum alfmt = AL_NONE;
  Uint8 *buffer = NULL;
  Uint32 buffer_length = 0;
  uint32_t bid = 0;

  if (!SDL_LoadWAV(
          asset_reader::ConvertFilePath(fileName, asset_type::Sound).c_str(),
          &spec, &buffer, &buffer_length)) {
    printf("Loading '%s' failed! %s\n", fileName.data(), SDL_GetError());
    return;
  }

  alGenBuffers(1, &bid);
  alBufferData(bid, detail::get_openal_format(&spec), buffer, buffer_length,
               spec.freq);
  buffers.emplace(fileName.data(), bid);
  SDL_FreeWAV(buffer);
}

void sound_system::UnloadAudioFile(std::string_view fileName) {
  std::string name = fileName.data();
  if (buffers.contains(name)) {
    alDeleteBuffers(1, &buffers[fileName.data()]);
    detail::check_openal_error("alDeleteBuffers");
    buffers.erase(name);
  }
}

uint32_t sound_system::NewSource(std::string_view name) {
  ALuint source;
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffers[name.data()]);
  return source;
}

void sound_system::DeleteSource(uint32_t sourceID) {
  alDeleteBuffers(1, &sourceID);
}
vector3 calculate(const quaternion &r) {
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

  if (m21 >= 0.99 && m21 <= 1.01) {
    tx = Math::Pi / 2.f;
    ty = 0;
    tz = Math::Atan2(m10, m00);
  } else if (m21 >= -1.01f && m21 <= -0.99f) {
    tx = -Math::Pi / 2.f;
    ty = 0;
    tz = Math::Atan2(m10, m00);
  } else {
    tx = std::asin(-m21);
    ty = Math::Atan2(m20, m22);
    tz = Math::Atan2(m01, m11);
  }

  return vector3(Math::ToDegrees(tx), Math::ToDegrees(ty), Math::ToDegrees(tz));
}
} // namespace nen