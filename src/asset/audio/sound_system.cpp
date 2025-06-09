// std
#include <vector>

// internal
#include "glm/trigonometric.hpp"
#include "sound_system.hpp"
#include <core/io/data_stream.hpp>
#include <math/math.hpp>

// external
#include <AL/al.h>
#include <AL/alc.h>
#include <SDL3/SDL_log.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace sinen {
glm::vec3 calculate(const glm::quat &r);
std::unordered_map<std::string, uint32_t> SoundSystem::buffers;
void *SoundSystem::device = nullptr;
void *SoundSystem::context = nullptr;
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
  if ((spec->channels == 1) && (spec->format == SDL_AUDIO_U8)) {
    return AL_FORMAT_MONO8;
  } else if ((spec->channels == 1) && (spec->format == SDL_AUDIO_S16)) {
    return AL_FORMAT_MONO16;
  } else if ((spec->channels == 2) && (spec->format == SDL_AUDIO_U8)) {
    return AL_FORMAT_STEREO8;
  } else if ((spec->channels == 2) && (spec->format == SDL_AUDIO_S16)) {
    return AL_FORMAT_STEREO16;
  } else if ((spec->channels == 1) && (spec->format == SDL_AUDIO_F32)) {
    return alIsExtensionPresent("AL_EXT_FLOAT32")
               ? alGetEnumValue("AL_FORMAT_MONO_FLOAT32")
               : AL_NONE;
  } else if ((spec->channels == 2) && (spec->format == SDL_AUDIO_F32)) {
    return alIsExtensionPresent("AL_EXT_FLOAT32")
               ? alGetEnumValue("AL_FORMAT_STEREO_FLOAT32")
               : AL_NONE;
  }
  return AL_NONE;
}
} // namespace detail

bool SoundSystem::initialize() {
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

void SoundSystem::shutdown() {
  for (auto &i : buffers) {
    alDeleteBuffers(1, &i.second);
  }
  alcMakeContextCurrent(NULL);
  alcDestroyContext((ALCcontext *)context);
  alcCloseDevice((ALCdevice *)device);
}

void SoundSystem::update(float deltaTime) {}

void SoundSystem::set_listener(const glm::vec3 &pos,
                               const glm::quat &direction) {
  alListener3f(AL_POSITION, pos.x, pos.y, pos.z);

  auto at = glm::vec3(0, 0, -1) * direction;
  auto up = glm::vec3(0, 1, 0) * direction;
  float ori[6] = {at.x, at.y, at.z, up.x, up.y, up.z};
  alListenerfv(AL_ORIENTATION, ori);
}

void SoundSystem::load(std::string_view fileName) {
  if (buffers.contains(fileName.data())) {
    return;
  }
  SDL_AudioSpec spec;
  ALenum alfmt = AL_NONE;
  Uint8 *buffer = NULL;
  Uint32 buffer_length = 0;
  uint32_t bid = 0;

  if (!SDL_LoadWAV(
          DataStream::ConvertFilePath(AssetType::Sound, fileName).c_str(),
          &spec, &buffer, &buffer_length)) {
    printf("Loading '%s' failed! %s\n", fileName.data(), SDL_GetError());
    return;
  }

  alGenBuffers(1, &bid);
  alBufferData(bid, detail::get_openal_format(&spec), buffer, buffer_length,
               spec.freq);
  buffers.emplace(fileName.data(), bid);
  SDL_free(buffer);
}

void SoundSystem::unload(std::string_view fileName) {
  std::string name = fileName.data();
  if (buffers.contains(name)) {
    alDeleteBuffers(1, &buffers[fileName.data()]);
    detail::check_openal_error("alDeleteBuffers");
    buffers.erase(name);
  }
}

uint32_t SoundSystem::new_source(std::string_view name) {
  ALuint source;
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, buffers[name.data()]);
  return source;
}

void SoundSystem::delete_source(uint32_t sourceID) {
  alDeleteBuffers(1, &sourceID);
}
glm::vec3 calculate(const glm::quat &r) {
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
    tx = Math::pi / 2.f;
    ty = 0;
    tz = Math::atan2(m10, m00);
  } else if (m21 >= -1.01f && m21 <= -0.99f) {
    tx = -Math::pi / 2.f;
    ty = 0;
    tz = Math::atan2(m10, m00);
  } else {
    tx = std::asin(-m21);
    ty = Math::atan2(m20, m22);
    tz = Math::atan2(m01, m11);
  }

  return glm::vec3(glm::degrees(tx), glm::degrees(ty), glm::degrees(tz));
}
} // namespace sinen
