// std
#include <vector>

// internal
#include "audio_system.hpp"
#include "glm/trigonometric.hpp"
#include <core/io/asset_io.hpp>
#include <math/math.hpp>

// external
#include <SDL3/SDL_log.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace sinen {
glm::vec3 calculate(const glm::quat &r);
std::unordered_map<std::string, uint32_t> AudioSystem::buffers;

bool AudioSystem::initialize() {

  auto resourceManagerConfig = ma_resource_manager_config_init();
  resourceManagerConfig.decodedFormat = ma_format_f32;
  resourceManagerConfig.decodedChannels = 2;
  resourceManagerConfig.decodedSampleRate = 48000;

  if (ma_resource_manager_init(&resourceManagerConfig, &data.resouceManager) !=
      MA_SUCCESS) {
    return false;
  }
  auto engineConfig = ma_engine_config_init();
  engineConfig.channels = 2;
  engineConfig.sampleRate = 48000;
  engineConfig.pResourceManager = &data.resouceManager;
  if (ma_engine_init(&engineConfig, &data.engine) != MA_SUCCESS) {
    return false;
  }
  if (ma_engine_start(&data.engine) != MA_SUCCESS) {
    return false;
  }
  return true;
}

void AudioSystem::shutdown() {
  ma_engine_uninit(&data.engine);
  ma_resource_manager_uninit(&data.resouceManager);
}

void AudioSystem::update(float deltaTime) {}

void AudioSystem::setListener(const glm::vec3 &pos,
                              const glm::quat &direction) {}

void AudioSystem::load(ma_sound *sound, std::string_view fileName) {

  auto path = AssetIO::getFilePath(AssetType::Sound, fileName);
  ma_sound_init_from_file(&data.engine, path.c_str(),
                          MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
                          nullptr, sound);
}
void AudioSystem::loadFromPath(std::string_view path) {}

void AudioSystem::unload(std::string_view fileName) {}

uint32_t AudioSystem::new_source(std::string_view name) { return 0; }

void AudioSystem::deleteSource(uint32_t sourceID) {}
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
