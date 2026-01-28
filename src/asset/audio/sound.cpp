// internal
#include "audio.hpp"
#include <asset/audio/sound.hpp>
#include <memory>
#include <platform/io/asset_io.hpp>

// external
#include "core/data/ptr.hpp"
#if 1
#define MINIAUDIO_IMPLEMENTATION
#endif
#include <miniaudio.h>

namespace sinen {
class SoundImpl : public Sound {
public:
  SoundImpl() : sound() {}
  virtual ~SoundImpl() override { ma_sound_uninit(&sound); }

  void load(StringView fileName) override {
    auto path = AssetIO::getFilePath(fileName);
    ma_sound_init_from_file(Audio::getEngine(), path.c_str(),
                            MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, nullptr,
                            nullptr, &sound);
  }
  void load(const Buffer &buffer) override {
    ma_decoder decoder;
    ma_decoder_config dcfg = ma_decoder_config_init(ma_format_f32, 2, 44100);
    ma_result r =
        ma_decoder_init_memory(buffer.data(), buffer.size(), &dcfg, &decoder);
    if (r != MA_SUCCESS) {
      return;
    }
    ma_sound_init_from_data_source(Audio::getEngine(), &decoder,
                                   MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC,
                                   nullptr, &sound);
  }
  void play() override { ma_sound_start(&sound); }

  void restart() override {
    ma_data_source_seek_to_pcm_frame(sound.pDataSource, 0);
  }

  void stop() override { ma_sound_stop(&sound); }
  void setLooping(bool looping) override {
    ma_sound_set_looping(&sound, looping);
  }

  void setVolume(float value) override { ma_sound_set_volume(&sound, value); }

  void setPitch(float value) override { ma_sound_set_pitch(&sound, value); }

  void setPosition(const Vec3 &pos) override {
    ma_sound_set_position(&sound, pos.x, pos.y, pos.z);
  }
  void setDirection(const Vec3 &dir) override {
    ma_sound_set_direction(&sound, dir.x, dir.y, dir.z);
  }
  bool isPlaying() const override { return ma_sound_is_playing(&sound); }
  bool isLooping() const override { return ma_sound_is_looping(&sound); }

  float getVolume() const override { return ma_sound_get_volume(&sound); }

  float getPitch() const override { return ma_sound_get_pitch(&sound); }

  Vec3 getPosition() const override {
    auto temp = ma_sound_get_position(&sound);
    return Vec3{temp.x, temp.y, temp.z};
  }
  Vec3 getDirection() const override {
    auto temp = ma_sound_get_direction(&sound);
    return Vec3{temp.x, temp.y, temp.z};
  }

private:
  ma_sound sound;
};
Ptr<Sound> Sound::create() { return makePtr<SoundImpl>(); }
} // namespace sinen

#include <asset/script/luaapi.hpp>
namespace sinen {
static int lSoundNew(lua_State *L) {
  udPushPtr<Sound>(L, Sound::create());
  return 1;
}
static int lSoundLoad(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    s->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2);
  s->load(buf);
  return 0;
}
static int lSoundPlay(lua_State *L) {
  udPtr<Sound>(L, 1)->play();
  return 0;
}
static int lSoundRestart(lua_State *L) {
  udPtr<Sound>(L, 1)->restart();
  return 0;
}
static int lSoundStop(lua_State *L) {
  udPtr<Sound>(L, 1)->stop();
  return 0;
}
static int lSoundSetLooping(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  bool looping = lua_toboolean(L, 2) != 0;
  s->setLooping(looping);
  return 0;
}
static int lSoundSetVolume(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setVolume(v);
  return 0;
}
static int lSoundSetPitch(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setPitch(v);
  return 0;
}
static int lSoundSetPosition(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  auto &p = udValue<Vec3>(L, 2);
  s->setPosition(p);
  return 0;
}
static int lSoundSetDirection(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1);
  auto &d = udValue<Vec3>(L, 2);
  s->setDirection(d);
  return 0;
}
static int lSoundIsPlaying(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1)->isPlaying());
  return 1;
}
static int lSoundIsLooping(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1)->isLooping());
  return 1;
}
static int lSoundGetVolume(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1)->getVolume());
  return 1;
}
static int lSoundGetPitch(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1)->getPitch());
  return 1;
}
static int lSoundGetPosition(lua_State *L) {
  udNewOwned<Vec3>(L, udPtr<Sound>(L, 1)->getPosition());
  return 1;
}
static int lSoundGetDirection(lua_State *L) {
  udNewOwned<Vec3>(L, udPtr<Sound>(L, 1)->getDirection());
  return 1;
}
void registerSound(lua_State *L) {
  luaL_newmetatable(L, Sound::metaTableName());
  luaPushcfunction2(L, udPtrGc<Sound>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lSoundLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lSoundPlay);
  lua_setfield(L, -2, "play");
  luaPushcfunction2(L, lSoundRestart);
  lua_setfield(L, -2, "restart");
  luaPushcfunction2(L, lSoundStop);
  lua_setfield(L, -2, "stop");
  luaPushcfunction2(L, lSoundSetLooping);
  lua_setfield(L, -2, "setLooping");
  luaPushcfunction2(L, lSoundSetVolume);
  lua_setfield(L, -2, "setVolume");
  luaPushcfunction2(L, lSoundSetPitch);
  lua_setfield(L, -2, "setPitch");
  luaPushcfunction2(L, lSoundSetPosition);
  lua_setfield(L, -2, "setPosition");
  luaPushcfunction2(L, lSoundSetDirection);
  lua_setfield(L, -2, "setDirection");
  luaPushcfunction2(L, lSoundIsPlaying);
  lua_setfield(L, -2, "isPlaying");
  luaPushcfunction2(L, lSoundIsLooping);
  lua_setfield(L, -2, "isLooping");
  luaPushcfunction2(L, lSoundGetVolume);
  lua_setfield(L, -2, "getVolume");
  luaPushcfunction2(L, lSoundGetPitch);
  lua_setfield(L, -2, "getPitch");
  luaPushcfunction2(L, lSoundGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lSoundGetDirection);
  lua_setfield(L, -2, "getDirection");
  lua_pop(L, 1);

  pushSnNamed(L, "Sound");
  luaPushcfunction2(L, lSoundNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
