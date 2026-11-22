#include "luaapi.hpp"
#include <audio/sound.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/external_file.hpp>

namespace sinen {
static int lSoundNew(lua_State *L) {
  if (lua_gettop(L) != 1) {
    return luaLError2(L, "sn.Sound.new expects exactly one source");
  }
  auto sound = Sound::create();
  bool loaded = false;
  if (lua_isstring(L, 1)) {
    const char *path = luaL_checkstring(L, 1);
    if (!AssetReader::exists(path)) {
      sound.reset();
      return luaLError2(L, "sn.Sound.new asset not found: %s", path);
    }
    loaded = sound->load(StringView(path));
  } else if (auto *file = udValueOrNull<ExternalFile>(L, 1)) {
    auto buffer = file->read();
    loaded = buffer && buffer->size() > 0 && sound->load(*buffer);
  } else {
    auto &buffer = udValue<Buffer>(L, 1);
    loaded = buffer.size() > 0 && sound->load(buffer);
  }
  if (!loaded) {
    sound.reset();
    return luaLError2(L, "sn.Sound.new failed to load the sound");
  }
  udPushPtr<Sound>(L, std::move(sound));
  return 1;
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
static auto lSoundSetVolume(Ptr<Sound> &s, float v) {
  s->setVolume(v);
  return;
}
static auto lSoundSetPitch(Ptr<Sound> &s, float v) {
  s->setPitch(v);
  return;
}
static auto lSoundSetPosition(Ptr<Sound> &s, Vec3 &p) {
  s->setPosition(p);
  return;
}
static auto lSoundSetDirection(Ptr<Sound> &s, Vec3 &d) {
  s->setDirection(d);
  return;
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
void registerSound(luau::table &sn) {
  auto sound = engineClass<Ptr<Sound>>(sn, Sound::metaTableName());

  sound.raw_method<lSoundPlay>("play");
  sound.raw_method<lSoundRestart>("restart");
  sound.raw_method<lSoundStop>("stop");
  sound.raw_method<lSoundSetLooping>("setLooping");
  sound.method<lSoundSetVolume>("setVolume");
  sound.method<lSoundSetPitch>("setPitch");
  sound.method<lSoundSetPosition>("setPosition");
  sound.method<lSoundSetDirection>("setDirection");
  sound.raw_method<lSoundIsPlaying>("isPlaying");
  sound.raw_method<lSoundIsLooping>("isLooping");
  sound.raw_method<lSoundGetVolume>("getVolume");
  sound.raw_method<lSoundGetPitch>("getPitch");
  sound.raw_method<lSoundGetPosition>("getPosition");
  sound.raw_method<lSoundGetDirection>("getDirection");

  sound.raw_static_function<lSoundNew>("new");
}
} // namespace sinen
