// internal
#include "audio.hpp"
#include <math/math.hpp>
#include <math/quaternion.hpp>
#include <platform/io/asset_io.hpp>


// external
#include <SDL3/SDL.h>
#if 0
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <miniaudio.h>

namespace sinen {
bool Audio::initialize() {

  auto resourceManagerConfig = ma_resource_manager_config_init();
  resourceManagerConfig.decodedFormat = ma_format_f32;
  resourceManagerConfig.decodedChannels = 2;
  resourceManagerConfig.decodedSampleRate = 48000;

  if (ma_resource_manager_init(&resourceManagerConfig, &data.resouceManager) !=
      MA_SUCCESS) {
    return false;
  }
  auto engineConfig = ma_engine_config_init();
  engineConfig.noDevice = MA_TRUE;
  engineConfig.channels = 2;
  engineConfig.sampleRate = 48000;
  engineConfig.pResourceManager = &data.resouceManager;
  if (ma_engine_init(&engineConfig, &data.engine) != MA_SUCCESS) {
    return false;
  }
  auto dataCallBack = [](void *userdata, SDL_AudioStream *stream,
                         int additional_amount, int total_amount) {
    if (additional_amount > 0) {
      if (auto pBuffer = SDL_stack_alloc(Uint8, additional_amount)) {
        (void)userdata;
        ma_uint32 bufferSizeInFrames =
            static_cast<ma_uint32>(additional_amount) /
            ma_get_bytes_per_frame(ma_format_f32,
                                   ma_engine_get_channels(&data.engine));
        ma_engine_read_pcm_frames(&data.engine, pBuffer, bufferSizeInFrames,
                                  nullptr);

        SDL_PutAudioStreamData(stream, pBuffer, additional_amount);
        SDL_stack_free(data);
      }
    }
  };
  SDL_AudioSpec spec{};
  spec.freq = ma_engine_get_sample_rate(&data.engine);
  spec.format = SDL_AUDIO_F32LE;
  spec.channels = ma_engine_get_channels(&data.engine);
  SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, dataCallBack, nullptr);
  if (!stream) {
    return false;
  }

  SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream));

  return true;
}

void Audio::shutdown() {
  ma_engine_uninit(&data.engine);
  ma_resource_manager_uninit(&data.resouceManager);
}
} // namespace sinen
