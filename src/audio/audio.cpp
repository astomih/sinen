// internal
#include "audio.hpp"
#include <math/math.hpp>
#include <math/quaternion.hpp>
#include <platform/io/asset_reader.hpp>

// external
#include <SDL3/SDL.h>
#if 0
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <miniaudio.h>

namespace sinen {
static ma_engine engine;
static ma_resource_manager resouceManager;
static bool resourceManagerInitialized = false;
static bool engineInitialized = false;
static SDL_AudioStream *audioStream = nullptr;
bool Audio::initialize() {

  auto resourceManagerConfig = ma_resource_manager_config_init();
  resourceManagerConfig.decodedFormat = ma_format_f32;
  resourceManagerConfig.decodedChannels = 2;
  resourceManagerConfig.decodedSampleRate = 48000;

  if (ma_resource_manager_init(&resourceManagerConfig, &resouceManager) !=
      MA_SUCCESS) {
    return false;
  }
  resourceManagerInitialized = true;
  auto engineConfig = ma_engine_config_init();
  engineConfig.noDevice = MA_TRUE;
  engineConfig.channels = 2;
  engineConfig.sampleRate = 48000;
  engineConfig.pResourceManager = &resouceManager;
  if (ma_engine_init(&engineConfig, &engine) != MA_SUCCESS) {
    ma_resource_manager_uninit(&resouceManager);
    resourceManagerInitialized = false;
    return false;
  }
  engineInitialized = true;
  auto dataCallBack = [](void *userdata, SDL_AudioStream *stream,
                         int additional_amount, int total_amount) {
    if (additional_amount > 0) {
      if (auto pBuffer = SDL_stack_alloc(Uint8, additional_amount)) {
        (void)userdata;
        ma_uint32 bufferSizeInFrames =
            static_cast<ma_uint32>(additional_amount) /
            ma_get_bytes_per_frame(ma_format_f32,
                                   ma_engine_get_channels(&engine));
        ma_engine_read_pcm_frames(&engine, pBuffer, bufferSizeInFrames,
                                  nullptr);

        SDL_PutAudioStreamData(stream, pBuffer, additional_amount);
        SDL_stack_free(pBuffer);
      }
    }
  };
  SDL_AudioSpec spec{};
  spec.freq = ma_engine_get_sample_rate(&engine);
  spec.format = SDL_AUDIO_F32LE;
  spec.channels = ma_engine_get_channels(&engine);
  audioStream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, dataCallBack, nullptr);
  if (!audioStream) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "Audio output stream unavailable: %s", SDL_GetError());
    return true;
  }

  SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

  return true;
}

void Audio::shutdown() {
  if (audioStream) {
    SDL_DestroyAudioStream(audioStream);
    audioStream = nullptr;
  }
  if (engineInitialized) {
    ma_engine_uninit(&engine);
    engineInitialized = false;
  }
  if (resourceManagerInitialized) {
    ma_resource_manager_uninit(&resouceManager);
    resourceManagerInitialized = false;
  }
}

void *Audio::getEngine() { return (void *)&engine; }
} // namespace sinen
