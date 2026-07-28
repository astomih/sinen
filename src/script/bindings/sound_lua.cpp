#include "luaapi.hpp"
#include <audio/sound.hpp>
#include <platform/io/asset_reader.hpp>

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
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "play", lSoundPlay);
  Binding::registerFunction(L, "restart", lSoundRestart);
  Binding::registerFunction(L, "stop", lSoundStop);
  Binding::registerFunction(L, "setLooping", lSoundSetLooping);
  Binding::registerFunction(L, "setVolume", lSoundSetVolume);
  Binding::registerFunction(L, "setPitch", lSoundSetPitch);
  Binding::registerFunction(L, "setPosition", lSoundSetPosition);
  Binding::registerFunction(L, "setDirection", lSoundSetDirection);
  Binding::registerFunction(L, "isPlaying", lSoundIsPlaying);
  Binding::registerFunction(L, "isLooping", lSoundIsLooping);
  Binding::registerFunction(L, "getVolume", lSoundGetVolume);
  Binding::registerFunction(L, "getPitch", lSoundGetPitch);
  Binding::registerFunction(L, "getPosition", lSoundGetPosition);
  Binding::registerFunction(L, "getDirection", lSoundGetDirection);
  lua_pop(L, 1);

  pushSnNamed(L, "Sound");
  Binding::registerFunction(L, "new", lSoundNew);
  lua_pop(L, 1);
}
} // namespace sinen
