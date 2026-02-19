#include "random.hpp"
#include <math/random.hpp>
#include <math/vector.hpp>
#include <random>
#include <script/luaapi.hpp>
namespace sinen {
std::mt19937 Random::sGenerator;
bool Random::initialize() {
  std::random_device rd;
  Random::seed(rd());
  return true;
}
void Random::shutdown() {}

void Random::seed(unsigned int seed) { sGenerator.seed(seed); }

float Random::getFloat() { return getRange(0.0f, 1.0f); }

float Random::getRange(float min, float max) {
  if (max < min) {
    std::swap(min, max);
  }
  std::uniform_real_distribution<float> dist(min, max);
  return dist(sGenerator);
}

int Random::getIntRange(int min, int max) {
  if (max < min) {
    std::swap(min, max);
  }
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(sGenerator);
  return a;
}

Vec2 Random::getVector(const Vec2 &min, const Vec2 &max) {
  Vec2 r = Vec2(getFloat(), getFloat());
  return min + (max - min) * r;
}

Vec3 Random::getVector(const Vec3 &min, const Vec3 &max) {
  Vec3 r = Vec3(getFloat(), getFloat(), getFloat());
  return min + (max - min) * r;
}

static int lRandomGetRange(lua_State *L) {
  float min = static_cast<float>(luaL_checknumber(L, 1));
  float max = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Random::getRange(min, max));
  return 1;
}
static int lRandomGetIntRange(lua_State *L) {
  int min = static_cast<int>(luaL_checkinteger(L, 1));
  int max = static_cast<int>(luaL_checkinteger(L, 2));
  lua_pushinteger(L, Random::getIntRange(min, max));
  return 1;
}
void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  luaPushcfunction2(L, lRandomGetRange);
  lua_setfield(L, -2, "getRange");
  luaPushcfunction2(L, lRandomGetIntRange);
  lua_setfield(L, -2, "getIntRange");
  lua_pop(L, 1);
}

} // namespace sinen
