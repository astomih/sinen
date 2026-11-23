#include <ocean/ocean.hpp>
#include <script/sinen_lua_module.h>

#include <lua.h>
#include <lualib.h>

#include <cmath>
#include <exception>
#include <limits>
#include <string_view>

namespace {
using namespace sinen::ocean;
constexpr const char *metatable = "sinen.plugin.ocean.Ocean";

template <int (*Function)(lua_State *)> int guarded(lua_State *L) {
  try {
    return Function(L);
  } catch (const std::exception &error) {
    lua_pushstring(L, error.what());
  }
  lua_error(L);
}

double number(lua_State *L, int table, const char *key, double fallback) {
  lua_getfield(L, table, key);
  const double value = lua_isnil(L, -1) ? fallback : luaL_checknumber(L, -1);
  lua_pop(L, 1);
  return value;
}

std::uint32_t integer(lua_State *L, int table, const char *key,
                      std::uint32_t fallback) {
  const double value = number(L, table, key, fallback);
  if (!std::isfinite(value) || value < 0 ||
      value > std::numeric_limits<std::uint32_t>::max() ||
      std::floor(value) != value)
    luaL_error(L, "ocean: %s must be a uint32 integer", key);
  return static_cast<std::uint32_t>(value);
}

WaveSystem wave(lua_State *L, int parent, const char *key, WaveSystem system) {
  lua_getfield(L, parent, key);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return system;
  }
  luaL_checktype(L, -1, LUA_TTABLE);
  const int table = lua_gettop(L);
  lua_getfield(L, table, "spectrum");
  if (!lua_isnil(L, -1)) {
    const std::string_view name = luaL_checkstring(L, -1);
    if (name == "phillips")
      system.spectrum = Spectrum::Phillips;
    else if (name == "jonswap")
      system.spectrum = Spectrum::Jonswap;
    else
      luaL_error(L, "ocean: spectrum must be 'phillips' or 'jonswap'");
  }
  lua_pop(L, 1);
  system.significantWaveHeight =
      number(L, table, "significantWaveHeight", system.significantWaveHeight);
  system.windSpeed = number(L, table, "windSpeed", system.windSpeed);
  system.direction = number(L, table, "direction", system.direction);
  system.peakPeriod = number(L, table, "peakPeriod", system.peakPeriod);
  system.peakEnhancement =
      number(L, table, "peakEnhancement", system.peakEnhancement);
  system.spread = number(L, table, "spread", system.spread);
  system.shortWaveDamping =
      number(L, table, "shortWaveDamping", system.shortWaveDamping);
  lua_pop(L, 1);
  return system;
}

Ocean &check(lua_State *L) {
  return **static_cast<Ocean **>(luaL_checkudata(L, 1, metatable));
}

int create(lua_State *L) {
  Settings settings;
  if (!lua_isnoneornil(L, 1)) {
    luaL_checktype(L, 1, LUA_TTABLE);
    settings.resolution = integer(L, 1, "resolution", settings.resolution);
    settings.seed = integer(L, 1, "seed", settings.seed);
    settings.patchSize = number(L, 1, "patchSize", settings.patchSize);
    settings.depth = number(L, 1, "depth", settings.depth);
    settings.gravity = number(L, 1, "gravity", settings.gravity);
    settings.choppiness = number(L, 1, "choppiness", settings.choppiness);
    settings.foamThreshold =
        number(L, 1, "foamThreshold", settings.foamThreshold);
    settings.foamGain = number(L, 1, "foamGain", settings.foamGain);
    settings.wind = wave(L, 1, "wind", settings.wind);
    settings.swell = wave(L, 1, "swell", settings.swell);
  }
  auto **box = static_cast<Ocean **>(
      lua_newuserdatadtor(L, sizeof(Ocean *), [](lua_State *, void *data) {
        delete *static_cast<Ocean **>(data);
      }));
  *box = nullptr;
  luaL_getmetatable(L, metatable);
  lua_setmetatable(L, -2);
  *box = new Ocean(settings);
  return 1;
}

int update(lua_State *L) {
  check(L).update(luaL_checknumber(L, 2));
  return 0;
}

int sample(lua_State *L) {
  const auto s =
      check(L).sample(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  lua_createtable(L, 0, 7);
  const auto field = [&](const char *key, float value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
  };
  field("height", s.height);
  field("displacementX", s.displacementX);
  field("displacementZ", s.displacementZ);
  field("normalX", s.normalX);
  field("normalY", s.normalY);
  field("normalZ", s.normalZ);
  field("foam", s.foam);
  return 1;
}

int createMeshBuffers(lua_State *L) {
  const auto &ocean = check(L);
  auto *vertices = static_cast<Vertex *>(
      lua_newbuffer(L, ocean.vertexCount() * sizeof(Vertex)));
  ocean.writeVertices({vertices, ocean.vertexCount()});
  auto *indices = static_cast<std::uint32_t *>(
      lua_newbuffer(L, ocean.indexCount() * sizeof(std::uint32_t)));
  ocean.writeIndices({indices, ocean.indexCount()});
  return 2;
}

int writeVertices(lua_State *L) {
  const auto &ocean = check(L);
  size_t size = 0;
  void *data = luaL_checkbuffer(L, 2, &size);
  if (size != ocean.vertexCount() * sizeof(Vertex))
    luaL_error(L, "ocean: incorrect vertex buffer size");
  ocean.writeVertices({static_cast<Vertex *>(data), ocean.vertexCount()});
  return 0;
}

int info(lua_State *L) {
  const auto &ocean = check(L);
  lua_createtable(L, 0, 4);
  lua_pushinteger(L, ocean.settings().resolution);
  lua_setfield(L, -2, "resolution");
  lua_pushnumber(L, ocean.settings().patchSize);
  lua_setfield(L, -2, "patchSize");
  lua_pushnumber(L, static_cast<double>(ocean.vertexCount()));
  lua_setfield(L, -2, "vertexCount");
  lua_pushnumber(L, static_cast<double>(ocean.indexCount()));
  lua_setfield(L, -2, "indexCount");
  return 1;
}

void function(lua_State *L, const char *name, lua_CFunction fn) {
  lua_pushcfunction(L, fn, name);
  lua_setfield(L, -2, name);
}
} // namespace

uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

int sinen_lua_module_open(lua_State *L) {
  luaL_newmetatable(L, metatable);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  function(L, "update", guarded<update>);
  function(L, "sample", guarded<sample>);
  function(L, "createMeshBuffers", guarded<createMeshBuffers>);
  function(L, "writeVertices", guarded<writeVertices>);
  function(L, "info", guarded<info>);
  lua_pop(L, 1);
  lua_newtable(L);
  function(L, "new", guarded<create>);
  lua_setfield(L, LUA_REGISTRYINDEX, SINEN_LUA_PLUGIN_REGISTRY_PREFIX "ocean");
  return 0;
}

int sinen_lua_module_close(lua_State *L) {
  lua_pushnil(L);
  lua_setfield(L, LUA_REGISTRYINDEX, SINEN_LUA_PLUGIN_REGISTRY_PREFIX "ocean");
  return 0;
}
