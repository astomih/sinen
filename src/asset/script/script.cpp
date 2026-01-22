// internal
#include "script.hpp"

#include <asset/asset.hpp>
#include <asset/shader/builtin_shader.hpp>
#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>
#include <core/event/event.hpp>
#include <core/logger/log.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <math/graph/bfs_grid.hpp>
#include <math/math.hpp>
#include <math/periodic.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/gamepad.hpp>
#include <platform/input/key_input.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/input/mouse.hpp>
#include <platform/io/arguments.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

#ifdef SINEN_USE_LUAU
#include <Luau/Compiler.h>
#include <lua.h>
#include <lualib.h>
#else
#include <lua.hpp>
#endif

#include <imgui.h>

#include <cstddef>
#include <cstring>
#include <format>
#include <utility>

namespace sinen {
void luaPushcfunction2(lua_State *L, lua_CFunction f) {
#ifdef SINEN_USE_LUAU
  lua_pushcfunction(L, f, "sn function");
#else
  lua_pushcfunction(L, f);
#endif
}
int luaLError2(lua_State *L, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
#ifdef SINEN_USE_LUAU
  luaL_error(L, va_list(ap));
  return 1;
#else
  return luaL_error(L, va_list(ap));
#endif
}
static int luaLRef2(lua_State *L, int idx) {
#ifdef SINEN_USE_LUAU
  return lua_ref(L, idx);
#else
  return luaL_ref(L, idx);
#endif
}
static void luaLUnref2(lua_State *L, int idx, int r) {
#ifdef SINEN_USE_LUAU
  lua_unref(L, r);
#else
  luaL_unref(L, idx, r);
#endif
}
auto alloc = [](void *ud, void *ptr, size_t osize, size_t nsize) -> void * {
  (void)ud;
  // free
  if (nsize == 0) {
    if (ptr)
      sinen::GlobalAllocator::get()->deallocate(ptr, osize);
    return nullptr;
  }

  if (ptr == nullptr) {
    return sinen::GlobalAllocator::get()->allocate(nsize);
  }

  void *nptr = sinen::GlobalAllocator::get()->allocate(nsize);
  if (!nptr) {
    return nullptr;
  }

  std::memcpy(nptr, ptr, (osize < nsize) ? osize : nsize);
  sinen::GlobalAllocator::get()->deallocate(ptr, osize);
  return nptr;
};

static String toStringTrim(double value) {
  String s(std::format("{}", value));

  auto dot = s.find('.');
  if (dot == String::npos) {
    return s + ".0";
  }

  bool allZero = true;
  for (size_t i = dot + 1; i < s.size(); ++i) {
    if (s[i] != '0') {
      allZero = false;
      break;
    }
  }

  if (allZero) {
    return s.substr(0, dot + 2);
  }

  s.erase(s.find_last_not_of('0') + 1);
  return s;
}
using TablePair = Array<std::pair<String, String>>;
static String convert(StringView name, const TablePair &p, bool isReturn) {
  String s;
  s = name.data();
  s += "{ ";
  if (isReturn) {
    s += "\n";
  }
  for (int i = 0; i < p.size(); i++) {
    auto &v = p[i];
    if (isReturn) {
      s += "\t";
    }
    s += v.first + " = " + v.second;
    if (i < p.size() - 1) {
      s += ", ";
    }
    if (isReturn) {
      s += "\n";
    }
  }
  s += " }";
  return s;
}
static lua_State *gLua = nullptr;
static int gSetupRef = LUA_NOREF;
static int gUpdateRef = LUA_NOREF;
static int gDrawRef = LUA_NOREF;
static auto vec3Str(const Vec3 &v) {
  TablePair p;
  p.emplace_back("x", toStringTrim(v.x));
  p.emplace_back("y", toStringTrim(v.y));
  p.emplace_back("z", toStringTrim(v.z));
  return convert("sn.Vec3", p, false);
};
static auto vec2Str(const Vec2 &v) {
  TablePair p;
  p.emplace_back("x", toStringTrim(v.x));
  p.emplace_back("y", toStringTrim(v.y));

  return convert("sn.Vec2", p, false);
};
static auto textureStr(const Texture &v) {
  TablePair p;
  p.emplace_back("isLoaded", v.texture ? "true" : "false");
  return convert("sn.Texture", p, false);
};
static auto transformStr(const Transform &v) {
  TablePair p;
  p.emplace_back("position", vec3Str(v.position));
  p.emplace_back("rotation", vec3Str(v.rotation));
  p.emplace_back("scale", vec3Str(v.scale));
  return convert("sn.Transform", p, true);
};

static auto colorStr(const Color &v) {
  TablePair p;
  p.emplace_back("r", toStringTrim(v.r));
  p.emplace_back("g", toStringTrim(v.g));
  p.emplace_back("b", toStringTrim(v.b));
  p.emplace_back("a", toStringTrim(v.a));
  return convert("sn.Color", p, false);
};

namespace {

template <class T> struct UdBox {
  bool owned = true;
  T *ptr = nullptr;
  alignas(T) std::byte storage[sizeof(T)];
};

static void *luaLTestudata2(lua_State *L, int idx, const char *tname) {
#ifdef SINEN_USE_LUAU
  void *p = lua_touserdata(L, idx);
  if (!p) {
    return nullptr;
  }
  if (!lua_getmetatable(L, idx)) {
    return nullptr;
  }
  luaL_getmetatable(L, tname);
  bool ok = lua_rawequal(L, -1, -2) != 0;
  lua_pop(L, 2);
  return ok ? p : nullptr;
#else
  return luaL_testudata(L, idx, tname);
#endif
}

template <class T> static T &udValue(lua_State *L, int idx, const char *mt) {
  auto *ud = static_cast<UdBox<T> *>(luaL_checkudata(L, idx, mt));
  return *ud->ptr;
}
template <class T>
static T *udValueOrNull(lua_State *L, int idx, const char *mt) {
  auto *ud = static_cast<UdBox<T> *>(luaLTestudata2(L, idx, mt));
  return ud ? ud->ptr : nullptr;
}
template <class T> static int udGc(lua_State *L) {
  auto *ud = static_cast<UdBox<T> *>(lua_touserdata(L, 1));
  if (ud && ud->owned && ud->ptr) {
    ud->ptr->~T();
    ud->ptr = nullptr;
  }
  return 0;
}
template <class T>
static UdBox<T> *udNewOwned(lua_State *L, const char *mt, T value) {
  void *mem = lua_newuserdata(L, sizeof(UdBox<T>));
  auto *ud = new (mem) UdBox<T>();
  ud->owned = true;
  ud->ptr = new (ud->storage) T(std::move(value));
  luaL_getmetatable(L, mt);
  lua_setmetatable(L, -2);
  return ud;
}
template <class T>
static UdBox<T> *udNewRef(lua_State *L, const char *mt, T *ref) {
  void *mem = lua_newuserdata(L, sizeof(UdBox<T>));
  auto *ud = new (mem) UdBox<T>();
  ud->owned = false;
  ud->ptr = ref;
  luaL_getmetatable(L, mt);
  lua_setmetatable(L, -2);
  return ud;
}

template <class T> static Ptr<T> &udPtr(lua_State *L, int idx, const char *mt) {
  return *static_cast<Ptr<T> *>(luaL_checkudata(L, idx, mt));
}
template <class T> static int udPtrGc(lua_State *L) {
  auto *ud = static_cast<Ptr<T> *>(lua_touserdata(L, 1));
  ud->~Ptr<T>();
  return 0;
}
template <class T>
static void udPushPtr(lua_State *L, const char *mt, Ptr<T> value) {
  void *mem = lua_newuserdata(L, sizeof(Ptr<T>));
  new (mem) Ptr<T>(std::move(value));
  luaL_getmetatable(L, mt);
  lua_setmetatable(L, -2);
}

static void pushSn(lua_State *L) { lua_getglobal(L, "sn"); }
static void ensureSn(lua_State *L) {
  lua_getglobal(L, "sn");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "sn");
  }
}
static void pushSnNamed(lua_State *L, const char *name) {
  ensureSn(L);
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, name);
  lua_remove(L, -2); // remove sn
}

static int luaPCallLogged(lua_State *L, int nargs, int nresults) {
  if (lua_pcall(L, nargs, nresults, 0) == LUA_OK) {
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  LogF::error("[lua error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return LUA_ERRRUN;
}

// metatable names
static constexpr const char *mtVec2 = "sn.Vec2";
static constexpr const char *mtVec3 = "sn.Vec3";
static constexpr const char *mtColor = "sn.Color";
static constexpr const char *mtRect = "sn.Rect";
static constexpr const char *mtTransform = "sn.Transform";
static constexpr const char *mtRay = "sn.Ray";
static constexpr const char *mtFont = "sn.Font";
static constexpr const char *mtTexture = "sn.Texture";
static constexpr const char *mtModel = "sn.Model";
static constexpr const char *mtBuffer = "sn.Buffer";
static constexpr const char *mtRendertexture = "sn.RenderTexture";
static constexpr const char *mtSound = "sn.Sound";
static constexpr const char *mtCamera = "sn.Camera";
static constexpr const char *mtCamerA2D = "sn.Camera2D";
static constexpr const char *mtAabb = "sn.AABB";
static constexpr const char *mtTimer = "sn.Timer";
static constexpr const char *mtCollider = "sn.Collider";
static constexpr const char *mtShader = "sn.Shader";
static constexpr const char *mtPipeline = "sn.GraphicsPipeline";
static constexpr const char *mtGrid = "sn.Grid";
static constexpr const char *mtBfsgrid = "sn.BFSGrid";
static constexpr const char *mtMouse = "sn.Mouse";

// -----------------
// Vec2
// -----------------
static int lVec2New(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec2>(L, mtVec2, Vec2(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec2>(L, mtVec2, Vec2(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Vec2>(L, mtVec2, Vec2(x, y));
  return 1;
}
static int lVec2Index(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, mtVec2);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, v.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, v.y);
    return 1;
  }
  luaL_getmetatable(L, mtVec2);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lVec2Newindex(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, mtVec2);
  const char *k = luaL_checkstring(L, 2);
  float value = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    v.x = value;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    v.y = value;
    return 0;
  }
  return luaLError2(L, "sn.Vec2: invalid field '%s'", k);
}
static int lVec2Add(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, mtVec2);
  auto &b = udValue<Vec2>(L, 2, mtVec2);
  udNewOwned<Vec2>(L, mtVec2, a + b);
  return 1;
}
static int lVec2Sub(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, mtVec2);
  auto &b = udValue<Vec2>(L, 2, mtVec2);
  udNewOwned<Vec2>(L, mtVec2, a - b);
  return 1;
}
static int lVec2Mul(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, mtVec2);
  auto &b = udValue<Vec2>(L, 2, mtVec2);
  udNewOwned<Vec2>(L, mtVec2, a * b);
  return 1;
}
static int lVec2Div(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, mtVec2);
  auto &b = udValue<Vec2>(L, 2, mtVec2);
  udNewOwned<Vec2>(L, mtVec2, a / b);
  return 1;
}
static int lVec2Tostring(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, mtVec2);
  String s = vec2Str(v);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int lVec2Copy(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, mtVec2);
  udNewOwned<Vec2>(L, mtVec2, v);
  return 1;
}
static int lVec2Length(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, mtVec2);
  lua_pushnumber(L, v.length());
  return 1;
}
static void registerVec2(lua_State *L) {
  luaL_newmetatable(L, mtVec2);
  luaPushcfunction2(L, udGc<Vec2>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lVec2Index);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lVec2Newindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lVec2Add);
  lua_setfield(L, -2, "__add");
  luaPushcfunction2(L, lVec2Sub);
  lua_setfield(L, -2, "__sub");
  luaPushcfunction2(L, lVec2Mul);
  lua_setfield(L, -2, "__mul");
  luaPushcfunction2(L, lVec2Div);
  lua_setfield(L, -2, "__div");
  luaPushcfunction2(L, lVec2Tostring);
  lua_setfield(L, -2, "__tostring");
  luaPushcfunction2(L, lVec2Copy);
  lua_setfield(L, -2, "copy");
  luaPushcfunction2(L, lVec2Length);
  lua_setfield(L, -2, "length");
  lua_pop(L, 1);

  pushSnNamed(L, "Vec2");
  luaPushcfunction2(L, lVec2New);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Vec3
// -----------------
static int lVec3New(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec3>(L, mtVec3, Vec3(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec3>(L, mtVec3, Vec3(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  float z = static_cast<float>(luaL_checknumber(L, 3));
  udNewOwned<Vec3>(L, mtVec3, Vec3(x, y, z));
  return 1;
}
static int lVec3Index(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, mtVec3);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, v.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, v.y);
    return 1;
  }
  if (std::strcmp(k, "z") == 0) {
    lua_pushnumber(L, v.z);
    return 1;
  }
  luaL_getmetatable(L, mtVec3);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lVec3Newindex(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, mtVec3);
  const char *k = luaL_checkstring(L, 2);
  float value = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    v.x = value;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    v.y = value;
    return 0;
  }
  if (std::strcmp(k, "z") == 0) {
    v.z = value;
    return 0;
  }
  return luaLError2(L, "sn.Vec3: invalid field '%s'", k);
}
static int lVec3Add(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, mtVec3);
  auto &b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, a + b);
  return 1;
}
static int lVec3Sub(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, mtVec3);
  auto &b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, a - b);
  return 1;
}
static int lVec3Mul(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, mtVec3);
  auto &b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, a * b);
  return 1;
}
static int lVec3Div(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, mtVec3);
  auto &b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, a / b);
  return 1;
}
static int lVec3Tostring(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, mtVec3);
  String s = vec3Str(v);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int lVec3Copy(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, v);
  return 1;
}
static int lVec3Length(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, mtVec3);
  lua_pushnumber(L, v.length());
  return 1;
}
static int lVec3Normalize(lua_State *L) {
  auto v = udValue<Vec3>(L, 1, mtVec3);
  v.normalize();
  udNewOwned<Vec3>(L, mtVec3, v);
  return 1;
}
static int lVec3Dot(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, mtVec3);
  auto b = udValue<Vec3>(L, 2, mtVec3);
  lua_pushnumber(L, Vec3::dot(a, b));
  return 1;
}
static int lVec3Cross(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, mtVec3);
  auto b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, Vec3::cross(a, b));
  return 1;
}
static int lVec3Lerp(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, mtVec3);
  auto b = udValue<Vec3>(L, 2, mtVec3);
  float value = luaL_checknumber(L, 3);
  udNewOwned<Vec3>(L, mtVec3, Vec3::lerp(a, b, value));
  return 1;
}
static int lVec3Reflect(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, mtVec3);
  auto b = udValue<Vec3>(L, 2, mtVec3);
  udNewOwned<Vec3>(L, mtVec3, Vec3::reflect(a, b));
  return 1;
}
static void registerVec3(lua_State *L) {
  luaL_newmetatable(L, mtVec3);
  luaPushcfunction2(L, udGc<Vec3>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lVec3Index);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lVec3Newindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lVec3Add);
  lua_setfield(L, -2, "__add");
  luaPushcfunction2(L, lVec3Sub);
  lua_setfield(L, -2, "__sub");
  luaPushcfunction2(L, lVec3Mul);
  lua_setfield(L, -2, "__mul");
  luaPushcfunction2(L, lVec3Div);
  lua_setfield(L, -2, "__div");
  luaPushcfunction2(L, lVec3Tostring);
  lua_setfield(L, -2, "__tostring");
  luaPushcfunction2(L, lVec3Copy);
  lua_setfield(L, -2, "copy");
  luaPushcfunction2(L, lVec3Length);
  lua_setfield(L, -2, "length");
  luaPushcfunction2(L, lVec3Normalize);
  lua_setfield(L, -2, "normalize");
  luaPushcfunction2(L, lVec3Dot);
  lua_setfield(L, -2, "dot");
  luaPushcfunction2(L, lVec3Cross);
  lua_setfield(L, -2, "cross");
  luaPushcfunction2(L, lVec3Lerp);
  lua_setfield(L, -2, "lerp");
  luaPushcfunction2(L, lVec3Reflect);
  lua_setfield(L, -2, "reflect");
  lua_pop(L, 1);

  pushSnNamed(L, "Vec3");
  luaPushcfunction2(L, lVec3New);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Color
// -----------------
static int lColorNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Color>(L, mtColor, Color(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Color>(L, mtColor, Color(v));
    return 1;
  }
  if (n == 2) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    float a = static_cast<float>(luaL_checknumber(L, 2));
    udNewOwned<Color>(L, mtColor, Color(v, a));
    return 1;
  }
  float r = static_cast<float>(luaL_checknumber(L, 1));
  float g = static_cast<float>(luaL_checknumber(L, 2));
  float b = static_cast<float>(luaL_checknumber(L, 3));
  float a = static_cast<float>(luaL_optnumber(L, 4, 1.0));
  udNewOwned<Color>(L, mtColor, Color(r, g, b, a));
  return 1;
}
static int lColorIndex(lua_State *L) {
  auto &c = udValue<Color>(L, 1, mtColor);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "r") == 0) {
    lua_pushnumber(L, c.r);
    return 1;
  }
  if (std::strcmp(k, "g") == 0) {
    lua_pushnumber(L, c.g);
    return 1;
  }
  if (std::strcmp(k, "b") == 0) {
    lua_pushnumber(L, c.b);
    return 1;
  }
  if (std::strcmp(k, "a") == 0) {
    lua_pushnumber(L, c.a);
    return 1;
  }
  luaL_getmetatable(L, mtColor);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lColorNewindex(lua_State *L) {
  auto &c = udValue<Color>(L, 1, mtColor);
  const char *k = luaL_checkstring(L, 2);
  float value = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "r") == 0) {
    c.r = value;
    return 0;
  }
  if (std::strcmp(k, "g") == 0) {
    c.g = value;
    return 0;
  }
  if (std::strcmp(k, "b") == 0) {
    c.b = value;
    return 0;
  }
  if (std::strcmp(k, "a") == 0) {
    c.a = value;
    return 0;
  }
  return luaLError2(L, "sn.Color: invalid field '%s'", k);
}
static int lColorTostring(lua_State *L) {
  auto &c = udValue<Color>(L, 1, mtColor);
  String s = colorStr(c);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerColor(lua_State *L) {
  luaL_newmetatable(L, mtColor);
  luaPushcfunction2(L, udGc<Color>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lColorIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lColorNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lColorTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Color");
  luaPushcfunction2(L, lColorNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lRayIndex(lua_State *L) {
  auto &r = udValue<Ray>(L, 1, mtRay);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "origin") == 0) {
    udNewRef<Vec3>(L, mtRay, &r.origin);
    return 1;
  }
  if (std::strcmp(k, "direction") == 0) {
    udNewRef<Vec3>(L, mtRay, &r.direction);
    return 1;
  }
  luaL_getmetatable(L, mtRay);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static void registerRay(lua_State *L) {
  luaL_newmetatable(L, mtRay);
  luaPushcfunction2(L, udGc<Ray>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lRayIndex);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  pushSnNamed(L, "Ray");
  lua_pop(L, 1);
}

// -----------------
// Camera / Camera2D
// -----------------
static int lCameraNew(lua_State *L) {
  udNewOwned<Camera>(L, mtCamera, Camera{});
  return 1;
}
static int lCameraLookat(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  auto &pos = udValue<Vec3>(L, 2, mtVec3);
  auto &target = udValue<Vec3>(L, 3, mtVec3);
  auto &up = udValue<Vec3>(L, 4, mtVec3);
  cam.lookat(pos, target, up);
  return 0;
}
static int lCameraPerspective(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  float fov = static_cast<float>(luaL_checknumber(L, 2));
  float aspect = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.perspective(fov, aspect, nearZ, farZ);
  return 0;
}
static int lCameraOrthographic(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  float w = static_cast<float>(luaL_checknumber(L, 2));
  float h = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.orthographic(w, h, nearZ, farZ);
  return 0;
}
static int lCameraGetPosition(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  udNewRef<Vec3>(L, mtVec3, &cam.getPosition());
  return 1;
}
static int lCameraGetTarget(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  udNewOwned<Vec3>(L, mtVec3, cam.getTarget());
  return 1;
}
static int lCameraGetUp(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  udNewOwned<Vec3>(L, mtVec3, cam.getUp());
  return 1;
}
static int lCameraIsAabbInFrustum(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  auto &aabb = udValue<AABB>(L, 2, mtAabb);
  lua_pushboolean(L, cam.isAABBInFrustum(aabb));
  return 1;
}
static int lCameraScreenToWorldRay(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  auto &screenPos = udValue<Vec2>(L, 2, mtVec2);

  udNewOwned<Ray>(L, mtRay, cam.screenToWorldRay(screenPos));
  return 1;
}
static void registerCamera(lua_State *L) {
  luaL_newmetatable(L, mtCamera);
  luaPushcfunction2(L, udGc<Camera>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lCameraLookat);
  lua_setfield(L, -2, "lookat");
  luaPushcfunction2(L, lCameraPerspective);
  lua_setfield(L, -2, "perspective");
  luaPushcfunction2(L, lCameraOrthographic);
  lua_setfield(L, -2, "orthographic");
  luaPushcfunction2(L, lCameraGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lCameraGetTarget);
  lua_setfield(L, -2, "getTarget");
  luaPushcfunction2(L, lCameraGetUp);
  lua_setfield(L, -2, "getUp");
  luaPushcfunction2(L, lCameraIsAabbInFrustum);
  lua_setfield(L, -2, "isAABBInFrustum");
  luaPushcfunction2(L, lCameraScreenToWorldRay);
  lua_setfield(L, -2, "screenToWorldRay");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera");
  luaPushcfunction2(L, lCameraNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lCamera2DNew(lua_State *L) {
  udNewOwned<Camera2D>(L, mtCamerA2D, Camera2D{});
  return 1;
}
static int lCamera2DSize(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  udNewOwned<Vec2>(L, mtVec2, cam.size());
  return 1;
}
static int lCamera2DHalf(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  udNewOwned<Vec2>(L, mtVec2, cam.half());
  return 1;
}
static int lCamera2DResize(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  auto &size = udValue<Vec2>(L, 2, mtVec2);
  cam.resize(size);
  return 0;
}
static int lCamera2DWindowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  udNewOwned<Vec2>(L, mtVec2, cam.windowRatio());
  return 1;
}
static int lCamera2DInvWindowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  udNewOwned<Vec2>(L, mtVec2, cam.invWindowRatio());
  return 1;
}
static void registerCamera2D(lua_State *L) {
  luaL_newmetatable(L, mtCamerA2D);
  luaPushcfunction2(L, udGc<Camera2D>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lCamera2DSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lCamera2DHalf);
  lua_setfield(L, -2, "half");
  luaPushcfunction2(L, lCamera2DResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lCamera2DWindowRatio);
  lua_setfield(L, -2, "windowRatio");
  luaPushcfunction2(L, lCamera2DInvWindowRatio);
  lua_setfield(L, -2, "invWindowRatio");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  luaPushcfunction2(L, lCamera2DNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// AABB / Timer / Collider
// -----------------
static int lAabbNew(lua_State *L) {
  udNewOwned<AABB>(L, mtAabb, AABB{});
  return 1;
}
static int lAabbIndex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, mtAabb);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "min") == 0) {
    udNewRef<Vec3>(L, mtVec3, &aabb.min);
    return 1;
  }
  if (std::strcmp(k, "max") == 0) {
    udNewRef<Vec3>(L, mtVec3, &aabb.max);
    return 1;
  }
  luaL_getmetatable(L, mtAabb);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lAabbNewindex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, mtAabb);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3, mtVec3);
  if (std::strcmp(k, "min") == 0) {
    aabb.min = v;
    return 0;
  }
  if (std::strcmp(k, "max") == 0) {
    aabb.max = v;
    return 0;
  }
  return luaLError2(L, "sn.AABB: invalid field '%s'", k);
}
static int lAabbUpdateWorld(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, mtAabb);
  auto &p = udValue<Vec3>(L, 2, mtVec3);
  auto &scale = udValue<Vec3>(L, 3, mtVec3);
  auto &local = udValue<AABB>(L, 4, mtAabb);
  aabb.updateWorld(p, scale, local);
  return 0;
}
static int lAabbIntersectsAabb(lua_State *L) {
  auto &a = udValue<AABB>(L, 1, mtAabb);
  auto &b = udValue<AABB>(L, 2, mtAabb);
  lua_pushboolean(L, a.intersectsAABB(b));
  return 1;
}
static void registerAABB(lua_State *L) {
  luaL_newmetatable(L, mtAabb);
  luaPushcfunction2(L, udGc<AABB>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lAabbIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lAabbNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lAabbUpdateWorld);
  lua_setfield(L, -2, "updateWorld");
  luaPushcfunction2(L, lAabbIntersectsAabb);
  lua_setfield(L, -2, "intersectsAABB");
  lua_pop(L, 1);

  pushSnNamed(L, "AABB");
  luaPushcfunction2(L, lAabbNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lTimerNew(lua_State *L) {
  if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
    float t = static_cast<float>(lua_tonumber(L, 1));
    udNewOwned<Timer>(L, mtTimer, Timer(t));
    return 1;
  }
  udNewOwned<Timer>(L, mtTimer, Timer());
  return 1;
}
static int lTimerStart(lua_State *L) {
  udValue<Timer>(L, 1, mtTimer).start();
  return 0;
}
static int lTimerStop(lua_State *L) {
  udValue<Timer>(L, 1, mtTimer).stop();
  return 0;
}
static int lTimerIsStarted(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1, mtTimer).isStarted());
  return 1;
}
static int lTimerSetTime(lua_State *L) {
  float ms = static_cast<float>(luaL_checknumber(L, 2));
  udValue<Timer>(L, 1, mtTimer).setTime(ms);
  return 0;
}
static int lTimerCheck(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1, mtTimer).check());
  return 1;
}
static void registerTimer(lua_State *L) {
  luaL_newmetatable(L, mtTimer);
  luaPushcfunction2(L, udGc<Timer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTimerStart);
  lua_setfield(L, -2, "start");
  luaPushcfunction2(L, lTimerStop);
  lua_setfield(L, -2, "stop");
  luaPushcfunction2(L, lTimerIsStarted);
  lua_setfield(L, -2, "isStarted");
  luaPushcfunction2(L, lTimerSetTime);
  lua_setfield(L, -2, "setTime");
  luaPushcfunction2(L, lTimerCheck);
  lua_setfield(L, -2, "check");
  lua_pop(L, 1);

  pushSnNamed(L, "Timer");
  luaPushcfunction2(L, lTimerNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lColliderNew(lua_State *L) {
  udNewOwned<Collider>(L, mtCollider, Collider{});
  return 1;
}
static int lColliderGetPosition(lua_State *L) {
  udNewOwned<Vec3>(L, mtVec3,
                   udValue<Collider>(L, 1, mtCollider).getPosition());
  return 1;
}
static int lColliderGetVelocity(lua_State *L) {
  udNewOwned<Vec3>(L, mtVec3,
                   udValue<Collider>(L, 1, mtCollider).getVelocity());
  return 1;
}
static int lColliderSetLinearVelocity(lua_State *L) {
  auto &c = udValue<Collider>(L, 1, mtCollider);
  auto &v = udValue<Vec3>(L, 2, mtVec3);
  c.setLinearVelocity(v);
  return 0;
}
static void registerCollider(lua_State *L) {
  luaL_newmetatable(L, mtCollider);
  luaPushcfunction2(L, udGc<Collider>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lColliderGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lColliderGetVelocity);
  lua_setfield(L, -2, "getVelocity");
  luaPushcfunction2(L, lColliderSetLinearVelocity);
  lua_setfield(L, -2, "setLinearVelocity");
  lua_pop(L, 1);

  pushSnNamed(L, "Collider");
  luaPushcfunction2(L, lColliderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Grid / BFSGrid
// -----------------
static int lGridNew(lua_State *L) {
  float w = static_cast<float>(luaL_checknumber(L, 1));
  float h = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Grid>(L, mtGrid,
                   Grid(static_cast<size_t>(w), static_cast<size_t>(h)));
  return 1;
}
static int lGridAt(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  lua_pushnumber(L, g.at(x - 1, y - 1));
  return 1;
}
static int lGridSet(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  float v = static_cast<float>(luaL_checknumber(L, 4));
  g.at(x - 1, y - 1) = v;
  return 0;
}
static int lGridWidth(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<Grid>(L, 1, mtGrid).width()));
  return 1;
}
static int lGridHeight(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<Grid>(L, 1, mtGrid).height()));
  return 1;
}
static int lGridSize(lua_State *L) {
  lua_pushinteger(L,
                  static_cast<lua_Integer>(udValue<Grid>(L, 1, mtGrid).size()));
  return 1;
}
static int lGridClear(lua_State *L) {
  udValue<Grid>(L, 1, mtGrid).clear();
  return 0;
}
static int lGridResize(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  size_t w = static_cast<size_t>(luaL_checkinteger(L, 2));
  size_t h = static_cast<size_t>(luaL_checkinteger(L, 3));
  g.resize(w, h);
  return 0;
}
static int lGridFill(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  g.fill(v);
  return 0;
}
static int lGridFillRect(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  auto &r = udValue<Rect>(L, 2, mtRect);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.fillRect(r, v);
  return 0;
}
static int lGridSetRow(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setRow(idx - 1, v);
  return 0;
}
static int lGridSetColumn(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setColumn(idx - 1, v);
  return 0;
}
static void registerGrid(lua_State *L) {
  luaL_newmetatable(L, mtGrid);
  luaPushcfunction2(L, udGc<Grid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lGridAt);
  lua_setfield(L, -2, "at");
  luaPushcfunction2(L, lGridSet);
  lua_setfield(L, -2, "set");
  luaPushcfunction2(L, lGridWidth);
  lua_setfield(L, -2, "width");
  luaPushcfunction2(L, lGridHeight);
  lua_setfield(L, -2, "height");
  luaPushcfunction2(L, lGridSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lGridClear);
  lua_setfield(L, -2, "clear");
  luaPushcfunction2(L, lGridResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lGridFill);
  lua_setfield(L, -2, "fill");
  luaPushcfunction2(L, lGridFillRect);
  lua_setfield(L, -2, "fillRect");
  luaPushcfunction2(L, lGridSetRow);
  lua_setfield(L, -2, "setRow");
  luaPushcfunction2(L, lGridSetColumn);
  lua_setfield(L, -2, "setColumn");
  lua_pop(L, 1);

  pushSnNamed(L, "Grid");
  luaPushcfunction2(L, lGridNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lBfsGridNew(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, mtGrid);
  udNewOwned<BFSGrid>(L, mtBfsgrid, BFSGrid(g));
  return 1;
}
static int lBfsGridWidth(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1, mtBfsgrid).width()));
  return 1;
}
static int lBfsGridHeight(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1, mtBfsgrid).height()));
  return 1;
}
static int lBfsGridFindPath(lua_State *L) {
  auto &b = udValue<BFSGrid>(L, 1, mtBfsgrid);
  auto &start = udValue<Vec2>(L, 2, mtVec2);
  auto &end = udValue<Vec2>(L, 3, mtVec2);
  lua_pushboolean(L, b.findPath(start, end));
  return 1;
}
static int lBfsGridTrace(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, udValue<BFSGrid>(L, 1, mtBfsgrid).trace());
  return 1;
}
static int lBfsGridTraceable(lua_State *L) {
  lua_pushboolean(L, udValue<BFSGrid>(L, 1, mtBfsgrid).traceable());
  return 1;
}
static int lBfsGridReset(lua_State *L) {
  udValue<BFSGrid>(L, 1, mtBfsgrid).reset();
  return 0;
}
static void registerBFSGrid(lua_State *L) {
  luaL_newmetatable(L, mtBfsgrid);
  luaPushcfunction2(L, udGc<BFSGrid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lBfsGridWidth);
  lua_setfield(L, -2, "width");
  luaPushcfunction2(L, lBfsGridHeight);
  lua_setfield(L, -2, "height");
  luaPushcfunction2(L, lBfsGridFindPath);
  lua_setfield(L, -2, "findPath");
  luaPushcfunction2(L, lBfsGridTrace);
  lua_setfield(L, -2, "trace");
  luaPushcfunction2(L, lBfsGridTraceable);
  lua_setfield(L, -2, "traceable");
  luaPushcfunction2(L, lBfsGridReset);
  lua_setfield(L, -2, "reset");
  lua_pop(L, 1);

  pushSnNamed(L, "BFSGrid");
  luaPushcfunction2(L, lBfsGridNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Buffer
// -----------------
static int lBufferNew(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  Array<void *> chunks;
  Array<size_t> chunkSizes;

  size_t n = lua_objlen(L, 1);
  chunks.reserve(n);
  chunkSizes.reserve(n);

  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));

    if (auto *v3 = udValueOrNull<Vec3>(L, -1, mtVec3)) {
      size_t s = sizeof(Vec3);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v3, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *v2 = udValueOrNull<Vec2>(L, -1, mtVec2)) {
      size_t s = sizeof(Vec2);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v2, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *cam = udValueOrNull<Camera>(L, -1, mtCamera)) {
      size_t s = sizeof(Mat4) * 2;
      void *p = GlobalAllocator::get()->allocate(s);
      auto view = cam->getView();
      auto proj = cam->getProjection();
      std::memcpy(p, &view, sizeof(Mat4));
      std::memcpy(reinterpret_cast<std::byte *>(p) + sizeof(Mat4), &proj,
                  sizeof(Mat4));
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (lua_isnumber(L, -1)) {
      float v = static_cast<float>(lua_tonumber(L, -1));
      size_t s = sizeof(float);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, &v, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    }

    lua_pop(L, 1);
  }

  size_t total = 0;
  for (auto s : chunkSizes) {
    total += s;
  }
  void *ptr = GlobalAllocator::get()->allocate(total);
  size_t offset = 0;
  for (size_t i = 0; i < chunks.size(); ++i) {
    std::memcpy(reinterpret_cast<std::byte *>(ptr) + offset, chunks[i],
                chunkSizes[i]);
    GlobalAllocator::get()->deallocate(chunks[i], chunkSizes[i]);
    offset += chunkSizes[i];
  }

  Buffer buffer(BufferType::Binary,
                Ptr<void>(ptr, Deleter<void>(GlobalAllocator::get(), total)),
                total);
  udNewOwned<Buffer>(L, mtBuffer, std::move(buffer));
  return 1;
}
static void registerBuffer(lua_State *L) {
  luaL_newmetatable(L, mtBuffer);
  luaPushcfunction2(L, udGc<Buffer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  pushSnNamed(L, "Buffer");
  luaPushcfunction2(L, lBufferNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Pivot / Rect
// -----------------
static void registerPivot(lua_State *L) {
  pushSnNamed(L, "Pivot");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopLeft));
  lua_setfield(L, -2, "TopLeft");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopCenter));
  lua_setfield(L, -2, "TopCenter");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopRight));
  lua_setfield(L, -2, "TopRight");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomLeft));
  lua_setfield(L, -2, "BottomLeft");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomCenter));
  lua_setfield(L, -2, "BottomCenter");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomRight));
  lua_setfield(L, -2, "BottomRight");
  lua_pop(L, 1);
}

static int lRectNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Rect>(L, mtRect, Rect());
    return 1;
  }
  if (n == 4) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    udNewOwned<Rect>(L, mtRect, Rect(x, y, w, h));
    return 1;
  }
  if (n == 2) {
    auto &pos = udValue<Vec2>(L, 1, mtVec2);
    auto &size = udValue<Vec2>(L, 2, mtVec2);
    udNewOwned<Rect>(L, mtRect, Rect(pos, size));
    return 1;
  }
  if (n == 5) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float w = static_cast<float>(luaL_checknumber(L, 4));
    float h = static_cast<float>(luaL_checknumber(L, 5));
    udNewOwned<Rect>(L, mtRect, Rect(pivot, x, y, w, h));
    return 1;
  }
  if (n == 3) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    auto &pos = udValue<Vec2>(L, 2, mtVec2);
    auto &size = udValue<Vec2>(L, 3, mtVec2);
    udNewOwned<Rect>(L, mtRect, Rect(pivot, pos, size));
    return 1;
  }
  return luaLError2(L, "sn.Rect.new: invalid arguments");
}

static int lRectIndex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1, mtRect);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, r.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, r.y);
    return 1;
  }
  if (std::strcmp(k, "width") == 0) {
    lua_pushnumber(L, r.width);
    return 1;
  }
  if (std::strcmp(k, "height") == 0) {
    lua_pushnumber(L, r.height);
    return 1;
  }
  luaL_getmetatable(L, mtRect);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lRectNewindex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1, mtRect);
  const char *k = luaL_checkstring(L, 2);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    r.x = v;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    r.y = v;
    return 0;
  }
  if (std::strcmp(k, "width") == 0) {
    r.width = v;
    return 0;
  }
  if (std::strcmp(k, "height") == 0) {
    r.height = v;
    return 0;
  }
  return luaLError2(L, "sn.Rect: invalid field '%s'", k);
}
static void registerRect(lua_State *L) {
  luaL_newmetatable(L, mtRect);
  luaPushcfunction2(L, udGc<Rect>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lRectIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRectNewindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  pushSnNamed(L, "Rect");
  luaPushcfunction2(L, lRectNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Transform
// -----------------
static int lTransformNew(lua_State *L) {
  udNewOwned<Transform>(L, mtTransform, Transform());
  return 1;
}
static int lTransformIndex(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, mtTransform);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "position") == 0) {
    udNewRef<Vec3>(L, mtVec3, &t.position);
    return 1;
  }
  if (std::strcmp(k, "rotation") == 0) {
    udNewRef<Vec3>(L, mtVec3, &t.rotation);
    return 1;
  }
  if (std::strcmp(k, "scale") == 0) {
    udNewRef<Vec3>(L, mtVec3, &t.scale);
    return 1;
  }
  luaL_getmetatable(L, mtTransform);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lTransformNewindex(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, mtTransform);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3, mtVec3);
  if (std::strcmp(k, "position") == 0) {
    t.position = v;
    return 0;
  }
  if (std::strcmp(k, "rotation") == 0) {
    t.rotation = v;
    return 0;
  }
  if (std::strcmp(k, "scale") == 0) {
    t.scale = v;
    return 0;
  }
  return luaLError2(L, "sn.Transform: invalid field '%s'", k);
}
static int lTransformTostring(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, mtTransform);
  String s = transformStr(t);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerTransform(lua_State *L) {
  luaL_newmetatable(L, mtTransform);
  luaPushcfunction2(L, udGc<Transform>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lTransformIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTransformNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lTransformTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Transform");
  luaPushcfunction2(L, lTransformNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Font (Ptr)
// -----------------
static int lFontNew(lua_State *L) {
  udPushPtr<Font>(L, mtFont, makePtr<Font>());
  return 1;
}
static int lFontLoad(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, mtFont);
  int n = lua_gettop(L);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  if (n == 2) {
    lua_pushboolean(L, font->load(point));
    return 1;
  }
  if (lua_isstring(L, 3)) {
    const char *path = luaL_checkstring(L, 3);
    lua_pushboolean(L, font->load(point, StringView(path)));
    return 1;
  }
  auto &buf = udValue<Buffer>(L, 3, mtBuffer);
  lua_pushboolean(L, font->load(point, buf));
  return 1;
}
static int lFontResize(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, mtFont);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  font->resize(point);
  return 0;
}
static int lFontRegion(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, mtFont);
  const char *text = luaL_checkstring(L, 2);
  int fontSize = static_cast<int>(luaL_checkinteger(L, 3));
  Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 4));
  auto &vec = udValue<Vec2>(L, 5, mtVec2);
  udNewOwned<Rect>(L, mtRect,
                   font->region(StringView(text), fontSize, pivot, vec));
  return 1;
}
static void registerFont(lua_State *L) {
  luaL_newmetatable(L, mtFont);
  luaPushcfunction2(L, udPtrGc<Font>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lFontLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lFontResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lFontRegion);
  lua_setfield(L, -2, "region");
  lua_pop(L, 1);

  pushSnNamed(L, "Font");
  luaPushcfunction2(L, lFontNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Texture / Model (Ptr)
// -----------------
static int lTextureNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udPushPtr<Texture>(L, mtTexture, makePtr<Texture>());
    return 1;
  }
  int w = static_cast<int>(luaL_checkinteger(L, 1));
  int h = static_cast<int>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, mtTexture, makePtr<Texture>(w, h));
  return 1;
}
static int lTextureLoad(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    tex->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, mtBuffer);
  tex->load(buf);
  return 0;
}
static int lTextureLoadCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  const char *path = luaL_checkstring(L, 2);
  tex->loadCubemap(StringView(path));
  return 0;
}
static int lTextureFill(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  auto &c = udValue<Color>(L, 2, mtColor);
  tex->fill(c);
  return 0;
}
static int lTextureCopy(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  udPushPtr<Texture>(L, mtTexture, makePtr<Texture>(tex->copy()));
  return 1;
}
static int lTextureSize(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  udNewOwned<Vec2>(L, mtVec2, tex->size());
  return 1;
}
static int lTextureTostring(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  String s = textureStr(*tex);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerTexture(lua_State *L) {
  luaL_newmetatable(L, mtTexture);
  luaPushcfunction2(L, udPtrGc<Texture>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTextureLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lTextureLoadCubemap);
  lua_setfield(L, -2, "loadCubemap");
  luaPushcfunction2(L, lTextureFill);
  lua_setfield(L, -2, "fill");
  luaPushcfunction2(L, lTextureCopy);
  lua_setfield(L, -2, "copy");
  luaPushcfunction2(L, lTextureSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lTextureTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Texture");
  luaPushcfunction2(L, lTextureNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lModelNew(lua_State *L) {
  udPushPtr<Model>(L, mtModel, makePtr<Model>());
  return 1;
}
static int lModelLoad(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    m->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, mtBuffer);
  m->load(buf);
  return 0;
}
static int lModelGetAabb(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  udNewOwned<AABB>(L, mtAabb, m->getAABB());
  return 1;
}
static int lModelLoadSprite(lua_State *L) {
  udPtr<Model>(L, 1, mtModel)->loadSprite();
  return 0;
}
static int lModelLoadBox(lua_State *L) {
  udPtr<Model>(L, 1, mtModel)->loadBox();
  return 0;
}
static int lModelGetBoneUniformBuffer(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  udNewOwned<Buffer>(L, mtBuffer, m->getBoneUniformBuffer());
  return 1;
}
static int lModelPlay(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  float start = static_cast<float>(luaL_checknumber(L, 2));
  m->play(start);
  return 0;
}
static int lModelUpdate(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  float dt = static_cast<float>(luaL_checknumber(L, 2));
  m->update(dt);
  return 0;
}
static int lModelHasTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  lua_pushboolean(L, m->hasTexture(k));
  return 1;
}
static int lModelGetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, mtTexture, makePtr<Texture>(m->getTexture(k)));
  return 1;
}
static int lModelSetTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  auto &t = udPtr<Texture>(L, 3, mtTexture);
  m->setTexture(k, *t);
  return 0;
}
static void registerModel(lua_State *L) {
  luaL_newmetatable(L, mtModel);
  luaPushcfunction2(L, udPtrGc<Model>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lModelLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lModelGetAabb);
  lua_setfield(L, -2, "getAABB");
  luaPushcfunction2(L, lModelLoadSprite);
  lua_setfield(L, -2, "loadSprite");
  luaPushcfunction2(L, lModelLoadBox);
  lua_setfield(L, -2, "loadBox");
  luaPushcfunction2(L, lModelGetBoneUniformBuffer);
  lua_setfield(L, -2, "getBoneUniformBuffer");
  luaPushcfunction2(L, lModelPlay);
  lua_setfield(L, -2, "play");
  luaPushcfunction2(L, lModelUpdate);
  lua_setfield(L, -2, "update");
  luaPushcfunction2(L, lModelHasTexture);
  lua_setfield(L, -2, "hasTexture");
  luaPushcfunction2(L, lModelGetTexture);
  lua_setfield(L, -2, "getTexture");
  luaPushcfunction2(L, lModelSetTexture);
  lua_setfield(L, -2, "setTexture");
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  luaPushcfunction2(L, lModelNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static void registerTextureKey(lua_State *L) {
  pushSnNamed(L, "TextureKey");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::BaseColor));
  lua_setfield(L, -2, "BaseColor");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Normal));
  lua_setfield(L, -2, "Normal");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::DiffuseRoughness));
  lua_setfield(L, -2, "DiffuseRoughness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Metalness));
  lua_setfield(L, -2, "Metalness");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::Emissive));
  lua_setfield(L, -2, "Emissive");
  lua_pushinteger(L, static_cast<lua_Integer>(TextureKey::LightMap));
  lua_setfield(L, -2, "LightMap");
  lua_pop(L, 1);
}

// -----------------
// RenderTexture / Sound / Shader / GraphicsPipeline (Ptr)
// -----------------
static int lRenderTextureNew(lua_State *L) {
  udPushPtr<RenderTexture>(L, mtRendertexture, makePtr<RenderTexture>());
  return 1;
}
static int lRenderTextureCreate(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, mtRendertexture);
  int w = static_cast<int>(luaL_checkinteger(L, 2));
  int h = static_cast<int>(luaL_checkinteger(L, 3));
  rt->create(w, h);
  return 0;
}
static void registerRenderTexture(lua_State *L) {
  luaL_newmetatable(L, mtRendertexture);
  luaPushcfunction2(L, udPtrGc<RenderTexture>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRenderTextureCreate);
  lua_setfield(L, -2, "create");
  lua_pop(L, 1);

  pushSnNamed(L, "RenderTexture");
  luaPushcfunction2(L, lRenderTextureNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lSoundNew(lua_State *L) {
  udPushPtr<Sound>(L, mtSound, makePtr<Sound>());
  return 1;
}
static int lSoundLoad(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    s->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, mtBuffer);
  s->load(buf);
  return 0;
}
static int lSoundPlay(lua_State *L) {
  udPtr<Sound>(L, 1, mtSound)->play();
  return 0;
}
static int lSoundRestart(lua_State *L) {
  udPtr<Sound>(L, 1, mtSound)->restart();
  return 0;
}
static int lSoundStop(lua_State *L) {
  udPtr<Sound>(L, 1, mtSound)->stop();
  return 0;
}
static int lSoundSetLooping(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  bool looping = lua_toboolean(L, 2) != 0;
  s->setLooping(looping);
  return 0;
}
static int lSoundSetVolume(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setVolume(v);
  return 0;
}
static int lSoundSetPitch(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setPitch(v);
  return 0;
}
static int lSoundSetPosition(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  auto &p = udValue<Vec3>(L, 2, mtVec3);
  s->setPosition(p);
  return 0;
}
static int lSoundSetDirection(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, mtSound);
  auto &d = udValue<Vec3>(L, 2, mtVec3);
  s->setDirection(d);
  return 0;
}
static int lSoundIsPlaying(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1, mtSound)->isPlaying());
  return 1;
}
static int lSoundIsLooping(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1, mtSound)->isLooping());
  return 1;
}
static int lSoundGetVolume(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1, mtSound)->getVolume());
  return 1;
}
static int lSoundGetPitch(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1, mtSound)->getPitch());
  return 1;
}
static int lSoundGetPosition(lua_State *L) {
  udNewOwned<Vec3>(L, mtVec3, udPtr<Sound>(L, 1, mtSound)->getPosition());
  return 1;
}
static int lSoundGetDirection(lua_State *L) {
  udNewOwned<Vec3>(L, mtVec3, udPtr<Sound>(L, 1, mtSound)->getDirection());
  return 1;
}
static void registerSound(lua_State *L) {
  luaL_newmetatable(L, mtSound);
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

static int lShaderNew(lua_State *L) {
  udPushPtr<Shader>(L, mtShader, makePtr<Shader>());
  return 1;
}
static int lShaderLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1, mtShader);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  int numUniformData = static_cast<int>(luaL_checkinteger(L, 4));
  s->load(StringView(name), stage, numUniformData);
  return 0;
}
static int lShaderCompileAndLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1, mtShader);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  s->compileAndLoad(StringView(name), stage);
  return 0;
}
static void registerShader(lua_State *L) {
  luaL_newmetatable(L, mtShader);
  luaPushcfunction2(L, udPtrGc<Shader>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lShaderLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lShaderCompileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  luaPushcfunction2(L, lShaderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int lPipelineNew(lua_State *L) {
  udPushPtr<GraphicsPipeline>(L, mtPipeline, makePtr<GraphicsPipeline>());
  return 1;
}
static int lPipelineSetVertexShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  auto &s = udPtr<Shader>(L, 2, mtShader);
  p->setVertexShader(*s);
  return 0;
}
static int lPipelineSetFragmentShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  auto &s = udPtr<Shader>(L, 2, mtShader);
  p->setFragmentShader(*s);
  return 0;
}
static int lPipelineSetEnableDepthTest(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableDepthTest(enable);
  return 0;
}
static int lPipelineSetEnableInstanced(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableInstanced(enable);
  return 0;
}
static int lPipelineSetEnableAnimation(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableAnimation(enable);
  return 0;
}
static int lPipelineSetEnableTangent(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableTangent(enable);
  return 0;
}
static int lPipelineBuild(lua_State *L) {
  udPtr<GraphicsPipeline>(L, 1, mtPipeline)->build();
  return 0;
}
static void registerPipeline(lua_State *L) {
  luaL_newmetatable(L, mtPipeline);
  luaPushcfunction2(L, udPtrGc<GraphicsPipeline>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lPipelineSetVertexShader);
  lua_setfield(L, -2, "setVertexShader");
  luaPushcfunction2(L, lPipelineSetFragmentShader);
  lua_setfield(L, -2, "setFragmentShader");
  luaPushcfunction2(L, lPipelineSetEnableDepthTest);
  lua_setfield(L, -2, "setEnableDepthTest");
  luaPushcfunction2(L, lPipelineSetEnableInstanced);
  lua_setfield(L, -2, "setEnableInstanced");
  luaPushcfunction2(L, lPipelineSetEnableAnimation);
  lua_setfield(L, -2, "setEnableAnimation");
  luaPushcfunction2(L, lPipelineSetEnableTangent);
  lua_setfield(L, -2, "setEnableTangent");
  luaPushcfunction2(L, lPipelineBuild);
  lua_setfield(L, -2, "build");
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  luaPushcfunction2(L, lPipelineNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// sn.* static modules
// -----------------
static int lArgumentsGetArgc(lua_State *L) {
  lua_pushinteger(L, Arguments::getArgc());
  return 1;
}
static int lArgumentsGetArgv(lua_State *L) {
  auto argv = Arguments::getArgv();
  lua_newtable(L);
  for (size_t i = 0; i < argv.size(); ++i) {
    lua_pushlstring(L, argv[i].data(), argv[i].size());
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
  return 1;
}
static void registerArguments(lua_State *L) {
  pushSnNamed(L, "Arguments");
  luaPushcfunction2(L, lArgumentsGetArgc);
  lua_setfield(L, -2, "getArgc");
  luaPushcfunction2(L, lArgumentsGetArgv);
  lua_setfield(L, -2, "getArgv");
  lua_pop(L, 1);
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
static void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  luaPushcfunction2(L, lRandomGetRange);
  lua_setfield(L, -2, "getRange");
  luaPushcfunction2(L, lRandomGetIntRange);
  lua_setfield(L, -2, "getIntRange");
  lua_pop(L, 1);
}

static int lWindowGetName(lua_State *L) {
  auto name = Window::name();
  lua_pushlstring(L, name.data(), name.size());
  return 1;
}
static int lWindowSize(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::size());
  return 1;
}
static int lWindowHalf(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::half());
  return 1;
}
static int lWindowResize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1, mtVec2);
  Window::resize(size);
  return 0;
}
static int lWindowSetFullscreen(lua_State *L) {
  bool fs = lua_toboolean(L, 1) != 0;
  Window::setFullscreen(fs);
  return 0;
}
static int lWindowRename(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  Window::rename(StringView(name));
  return 0;
}
static int lWindowResized(lua_State *L) {
  lua_pushboolean(L, Window::resized());
  return 1;
}
static int lWindowRect(lua_State *L) {
  udNewOwned<Rect>(L, mtRect, Window::rect());
  return 1;
}
static int lWindowTopLeft(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::topLeft());
  return 1;
}
static int lWindowTopCenter(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::topCenter());
  return 1;
}
static int lWindowTopRight(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::topRight());
  return 1;
}
static int lWindowBottomLeft(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::bottomLeft());
  return 1;
}
static int lWindowBottomCenter(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::bottomCenter());
  return 1;
}
static int lWindowBottomRight(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Window::bottomRight());
  return 1;
}
static void registerWindow(lua_State *L) {
  pushSnNamed(L, "Window");
  luaPushcfunction2(L, lWindowGetName);
  lua_setfield(L, -2, "getName");
  luaPushcfunction2(L, lWindowSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lWindowHalf);
  lua_setfield(L, -2, "half");
  luaPushcfunction2(L, lWindowResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lWindowSetFullscreen);
  lua_setfield(L, -2, "setFullscreen");
  luaPushcfunction2(L, lWindowRename);
  lua_setfield(L, -2, "rename");
  luaPushcfunction2(L, lWindowResized);
  lua_setfield(L, -2, "resized");
  luaPushcfunction2(L, lWindowRect);
  lua_setfield(L, -2, "rect");
  luaPushcfunction2(L, lWindowTopLeft);
  lua_setfield(L, -2, "topLeft");
  luaPushcfunction2(L, lWindowTopCenter);
  lua_setfield(L, -2, "topCenter");
  luaPushcfunction2(L, lWindowTopRight);
  lua_setfield(L, -2, "topRight");
  luaPushcfunction2(L, lWindowBottomLeft);
  lua_setfield(L, -2, "bottomLeft");
  luaPushcfunction2(L, lWindowBottomCenter);
  lua_setfield(L, -2, "bottomCenter");
  luaPushcfunction2(L, lWindowBottomRight);
  lua_setfield(L, -2, "bottomRight");
  lua_pop(L, 1);
}

static int lPhysicsCreateBoxCollider(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, mtTransform);
  bool isStatic = lua_toboolean(L, 2) != 0;
  udNewOwned<Collider>(L, mtCollider, Physics::createBoxCollider(t, isStatic));
  return 1;
}
static int lPhysicsCreateSphereCollider(lua_State *L) {
  auto &pos = udValue<Vec3>(L, 1, mtVec3);
  float radius = static_cast<float>(luaL_checknumber(L, 2));
  bool isStatic = lua_toboolean(L, 3) != 0;
  udNewOwned<Collider>(L, mtCollider,
                       Physics::createSphereCollider(pos, radius, isStatic));
  return 1;
}
static int lPhysicsCreateCylinderCollider(lua_State *L) {
  auto &pos = udValue<Vec3>(L, 1, mtVec3);
  auto &rot = udValue<Vec3>(L, 2, mtVec3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 3));
  float radius = static_cast<float>(luaL_checknumber(L, 4));
  bool isStatic = lua_toboolean(L, 5) != 0;
  udNewOwned<Collider>(
      L, mtCollider,
      Physics::createCylinderCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}
static int lPhysicsAddCollider(lua_State *L) {
  auto &c = udValue<Collider>(L, 1, mtCollider);
  bool active = lua_toboolean(L, 2) != 0;
  Physics::addCollider(c, active);
  return 0;
}
static void registerPhysics(lua_State *L) {
  pushSnNamed(L, "Physics");
  luaPushcfunction2(L, lPhysicsCreateBoxCollider);
  lua_setfield(L, -2, "createBoxCollider");
  luaPushcfunction2(L, lPhysicsCreateSphereCollider);
  lua_setfield(L, -2, "createSphereCollider");
  luaPushcfunction2(L, lPhysicsCreateCylinderCollider);
  lua_setfield(L, -2, "createCylinderCollider");
  luaPushcfunction2(L, lPhysicsAddCollider);
  lua_setfield(L, -2, "addCollider");
  lua_pop(L, 1);
}

static void registerShaderStage(lua_State *L) {
  pushSnNamed(L, "ShaderStage");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Vertex));
  lua_setfield(L, -2, "Vertex");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Fragment));
  lua_setfield(L, -2, "Fragment");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Compute));
  lua_setfield(L, -2, "Compute");
  lua_pop(L, 1);
}

static int lBuiltinShaderGetDefaultVs(lua_State *L) {
  udPushPtr<Shader>(L, mtShader,
                    makePtr<Shader>(BuiltinShader::getDefaultVS()));
  return 1;
}
static int lBuiltinShaderGetDefaultFs(lua_State *L) {
  udPushPtr<Shader>(L, mtShader,
                    makePtr<Shader>(BuiltinShader::getDefaultFS()));
  return 1;
}
static int lBuiltinShaderGetDefaultInstancedVs(lua_State *L) {
  udPushPtr<Shader>(L, mtShader,
                    makePtr<Shader>(BuiltinShader::getDefaultInstancedVS()));
  return 1;
}
static int lBuiltinShaderGetCubemapVs(lua_State *L) {
  udPushPtr<Shader>(L, mtShader,
                    makePtr<Shader>(BuiltinShader::getCubemapVS()));
  return 1;
}
static int lBuiltinShaderGetCubemapFs(lua_State *L) {
  udPushPtr<Shader>(L, mtShader,
                    makePtr<Shader>(BuiltinShader::getCubemapFS()));
  return 1;
}
static void registerBuiltinShader(lua_State *L) {
  pushSnNamed(L, "BuiltinShader");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultVs);
  lua_setfield(L, -2, "getDefaultVS");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultFs);
  lua_setfield(L, -2, "getDefaultFS");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultInstancedVs);
  lua_setfield(L, -2, "getDefaultInstancedVS");
  luaPushcfunction2(L, lBuiltinShaderGetCubemapVs);
  lua_setfield(L, -2, "getCubemapVS");
  luaPushcfunction2(L, lBuiltinShaderGetCubemapFs);
  lua_setfield(L, -2, "getCubemapFS");
  lua_pop(L, 1);
}

static int lBuiltinPipelineGetDefault3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, mtPipeline,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault3D()));
  return 1;
}
static int lBuiltinPipelineGetInstanced3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, mtPipeline,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getInstanced3D()));
  return 1;
}
static int lBuiltinPipelineGetDefault2D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, mtPipeline,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault2D()));
  return 1;
}
static int lBuiltinPipelineGetCubemap(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, mtPipeline, makePtr<GraphicsPipeline>(BuiltinPipeline::getCubemap()));
  return 1;
}
static void registerBuiltinPipeline(lua_State *L) {
  pushSnNamed(L, "BuiltinPipeline");
  luaPushcfunction2(L, lBuiltinPipelineGetDefault3D);
  lua_setfield(L, -2, "getDefault3D");
  luaPushcfunction2(L, lBuiltinPipelineGetInstanced3D);
  lua_setfield(L, -2, "getInstanced3D");
  luaPushcfunction2(L, lBuiltinPipelineGetDefault2D);
  lua_setfield(L, -2, "getDefault2D");
  luaPushcfunction2(L, lBuiltinPipelineGetCubemap);
  lua_setfield(L, -2, "getCubemap");
  lua_pop(L, 1);
}

static int lEventQuit(lua_State *L) {
  (void)L;
  Event::quit();
  return 0;
}
static void registerEvent(lua_State *L) {
  pushSnNamed(L, "Event");
  luaPushcfunction2(L, lEventQuit);
  lua_setfield(L, -2, "quit");
  lua_pop(L, 1);
}

static int lKeyboardIsPressed(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isPressed(key));
  return 1;
}
static int lKeyboardIsReleased(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isReleased(key));
  return 1;
}
static int lKeyboardIsDown(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isDown(key));
  return 1;
}
static void registerKeyboard(lua_State *L) {
  pushSnNamed(L, "Keyboard");
  luaPushcfunction2(L, lKeyboardIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lKeyboardIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lKeyboardIsDown);
  lua_setfield(L, -2, "isDown");

  lua_pushinteger(L, static_cast<int>(Keyboard::A));
  lua_setfield(L, -2, "A");
  lua_pushinteger(L, static_cast<int>(Keyboard::B));
  lua_setfield(L, -2, "B");
  lua_pushinteger(L, static_cast<int>(Keyboard::C));
  lua_setfield(L, -2, "C");
  lua_pushinteger(L, static_cast<int>(Keyboard::D));
  lua_setfield(L, -2, "D");
  lua_pushinteger(L, static_cast<int>(Keyboard::E));
  lua_setfield(L, -2, "E");
  lua_pushinteger(L, static_cast<int>(Keyboard::F));
  lua_setfield(L, -2, "F");
  lua_pushinteger(L, static_cast<int>(Keyboard::G));
  lua_setfield(L, -2, "G");
  lua_pushinteger(L, static_cast<int>(Keyboard::H));
  lua_setfield(L, -2, "H");
  lua_pushinteger(L, static_cast<int>(Keyboard::I));
  lua_setfield(L, -2, "I");
  lua_pushinteger(L, static_cast<int>(Keyboard::J));
  lua_setfield(L, -2, "J");
  lua_pushinteger(L, static_cast<int>(Keyboard::K));
  lua_setfield(L, -2, "K");
  lua_pushinteger(L, static_cast<int>(Keyboard::L));
  lua_setfield(L, -2, "L");
  lua_pushinteger(L, static_cast<int>(Keyboard::M));
  lua_setfield(L, -2, "M");
  lua_pushinteger(L, static_cast<int>(Keyboard::N));
  lua_setfield(L, -2, "N");
  lua_pushinteger(L, static_cast<int>(Keyboard::O));
  lua_setfield(L, -2, "O");
  lua_pushinteger(L, static_cast<int>(Keyboard::P));
  lua_setfield(L, -2, "P");
  lua_pushinteger(L, static_cast<int>(Keyboard::Q));
  lua_setfield(L, -2, "Q");
  lua_pushinteger(L, static_cast<int>(Keyboard::R));
  lua_setfield(L, -2, "R");
  lua_pushinteger(L, static_cast<int>(Keyboard::S));
  lua_setfield(L, -2, "S");
  lua_pushinteger(L, static_cast<int>(Keyboard::T));
  lua_setfield(L, -2, "T");
  lua_pushinteger(L, static_cast<int>(Keyboard::U));
  lua_setfield(L, -2, "U");
  lua_pushinteger(L, static_cast<int>(Keyboard::V));
  lua_setfield(L, -2, "V");
  lua_pushinteger(L, static_cast<int>(Keyboard::W));
  lua_setfield(L, -2, "W");
  lua_pushinteger(L, static_cast<int>(Keyboard::X));
  lua_setfield(L, -2, "X");
  lua_pushinteger(L, static_cast<int>(Keyboard::Y));
  lua_setfield(L, -2, "Y");
  lua_pushinteger(L, static_cast<int>(Keyboard::Z));
  lua_setfield(L, -2, "Z");

  lua_pushinteger(L, static_cast<int>(Keyboard::Key0));
  lua_setfield(L, -2, "key0");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key1));
  lua_setfield(L, -2, "key1");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key2));
  lua_setfield(L, -2, "key2");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key3));
  lua_setfield(L, -2, "key3");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key4));
  lua_setfield(L, -2, "key4");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key5));
  lua_setfield(L, -2, "key5");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key6));
  lua_setfield(L, -2, "key6");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key7));
  lua_setfield(L, -2, "key7");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key8));
  lua_setfield(L, -2, "key8");
  lua_pushinteger(L, static_cast<int>(Keyboard::Key9));
  lua_setfield(L, -2, "key9");

  lua_pushinteger(L, static_cast<int>(Keyboard::F1));
  lua_setfield(L, -2, "F1");
  lua_pushinteger(L, static_cast<int>(Keyboard::F2));
  lua_setfield(L, -2, "F2");
  lua_pushinteger(L, static_cast<int>(Keyboard::F3));
  lua_setfield(L, -2, "F3");
  lua_pushinteger(L, static_cast<int>(Keyboard::F4));
  lua_setfield(L, -2, "F4");
  lua_pushinteger(L, static_cast<int>(Keyboard::F5));
  lua_setfield(L, -2, "F5");
  lua_pushinteger(L, static_cast<int>(Keyboard::F6));
  lua_setfield(L, -2, "F6");
  lua_pushinteger(L, static_cast<int>(Keyboard::F7));
  lua_setfield(L, -2, "F7");
  lua_pushinteger(L, static_cast<int>(Keyboard::F8));
  lua_setfield(L, -2, "F8");
  lua_pushinteger(L, static_cast<int>(Keyboard::F9));
  lua_setfield(L, -2, "F9");
  lua_pushinteger(L, static_cast<int>(Keyboard::F10));
  lua_setfield(L, -2, "F10");
  lua_pushinteger(L, static_cast<int>(Keyboard::F11));
  lua_setfield(L, -2, "F11");
  lua_pushinteger(L, static_cast<int>(Keyboard::F12));
  lua_setfield(L, -2, "F12");

  lua_pushinteger(L, static_cast<int>(Keyboard::UP));
  lua_setfield(L, -2, "UP");
  lua_pushinteger(L, static_cast<int>(Keyboard::DOWN));
  lua_setfield(L, -2, "DOWN");
  lua_pushinteger(L, static_cast<int>(Keyboard::LEFT));
  lua_setfield(L, -2, "LEFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::RIGHT));
  lua_setfield(L, -2, "RIGHT");
  lua_pushinteger(L, static_cast<int>(Keyboard::ESCAPE));
  lua_setfield(L, -2, "ESCAPE");
  lua_pushinteger(L, static_cast<int>(Keyboard::SPACE));
  lua_setfield(L, -2, "SPACE");
  lua_pushinteger(L, static_cast<int>(Keyboard::RETURN));
  lua_setfield(L, -2, "ENTER");
  lua_pushinteger(L, static_cast<int>(Keyboard::BACKSPACE));
  lua_setfield(L, -2, "BACKSPACE");
  lua_pushinteger(L, static_cast<int>(Keyboard::TAB));
  lua_setfield(L, -2, "TAB");
  lua_pushinteger(L, static_cast<int>(Keyboard::LSHIFT));
  lua_setfield(L, -2, "LSHIFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::RSHIFT));
  lua_setfield(L, -2, "RSHIFT");
  lua_pushinteger(L, static_cast<int>(Keyboard::LCTRL));
  lua_setfield(L, -2, "LCTRL");
  lua_pushinteger(L, static_cast<int>(Keyboard::RCTRL));
  lua_setfield(L, -2, "RCTRL");
  lua_pushinteger(L, static_cast<int>(Keyboard::ALTERASE));
  lua_setfield(L, -2, "ALT");

  lua_pop(L, 1);
}

static int lGamepadIsPressed(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isPressed(btn));
  return 1;
}
static int lGamepadIsReleased(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isReleased(btn));
  return 1;
}
static int lGamepadIsDown(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isDown(btn));
  return 1;
}
static int lGamepadGetLeftStick(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, GamePad::getLeftStick());
  return 1;
}
static int lGamepadGetRightStick(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, GamePad::getRightStick());
  return 1;
}
static int lGamepadIsConnected(lua_State *L) {
  lua_pushboolean(L, GamePad::isConnected());
  return 1;
}
static void registerGamepad(lua_State *L) {
  pushSnNamed(L, "Gamepad");
  luaPushcfunction2(L, lGamepadIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lGamepadIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lGamepadIsDown);
  lua_setfield(L, -2, "isDown");
  luaPushcfunction2(L, lGamepadGetLeftStick);
  lua_setfield(L, -2, "getLeftStick");
  luaPushcfunction2(L, lGamepadGetRightStick);
  lua_setfield(L, -2, "getRightStick");
  luaPushcfunction2(L, lGamepadIsConnected);
  lua_setfield(L, -2, "isConnected");

  lua_pushinteger(L, static_cast<int>(GamePad::INVALID));
  lua_setfield(L, -2, "INVALID");
  lua_pushinteger(L, static_cast<int>(GamePad::A));
  lua_setfield(L, -2, "A");
  lua_pushinteger(L, static_cast<int>(GamePad::B));
  lua_setfield(L, -2, "B");
  lua_pushinteger(L, static_cast<int>(GamePad::X));
  lua_setfield(L, -2, "X");
  lua_pushinteger(L, static_cast<int>(GamePad::Y));
  lua_setfield(L, -2, "Y");
  lua_pushinteger(L, static_cast<int>(GamePad::BACK));
  lua_setfield(L, -2, "BACK");
  lua_pushinteger(L, static_cast<int>(GamePad::GUIDE));
  lua_setfield(L, -2, "GUIDE");
  lua_pushinteger(L, static_cast<int>(GamePad::START));
  lua_setfield(L, -2, "START");
  lua_pushinteger(L, static_cast<int>(GamePad::LEFTSTICK));
  lua_setfield(L, -2, "LEFTSTICK");
  lua_pushinteger(L, static_cast<int>(GamePad::RIGHTSTICK));
  lua_setfield(L, -2, "RIGHTSTICK");
  lua_pushinteger(L, static_cast<int>(GamePad::LEFTSHOULDER));
  lua_setfield(L, -2, "LEFTSHOULDER");
  lua_pushinteger(L, static_cast<int>(GamePad::RIGHTSHOULDER));
  lua_setfield(L, -2, "RIGHTSHOULDER");
  lua_pushinteger(L, static_cast<int>(GamePad::DPAD_UP));
  lua_setfield(L, -2, "DPAD_UP");
  lua_pushinteger(L, static_cast<int>(GamePad::DPAD_DOWN));
  lua_setfield(L, -2, "DPAD_DOWN");
  lua_pushinteger(L, static_cast<int>(GamePad::DPAD_LEFT));
  lua_setfield(L, -2, "DPAD_LEFT");
  lua_pushinteger(L, static_cast<int>(GamePad::DPAD_RIGHT));
  lua_setfield(L, -2, "DPAD_RIGHT");
  lua_pushinteger(L, static_cast<int>(GamePad::MISC1));
  lua_setfield(L, -2, "MISC1");
  lua_pushinteger(L, static_cast<int>(GamePad::PADDLE1));
  lua_setfield(L, -2, "PADDLE1");
  lua_pushinteger(L, static_cast<int>(GamePad::PADDLE2));
  lua_setfield(L, -2, "PADDLE2");
  lua_pushinteger(L, static_cast<int>(GamePad::PADDLE3));
  lua_setfield(L, -2, "PADDLE3");
  lua_pushinteger(L, static_cast<int>(GamePad::PADDLE4));
  lua_setfield(L, -2, "PADDLE4");
  lua_pushinteger(L, static_cast<int>(GamePad::TOUCHPAD));
  lua_setfield(L, -2, "TOUCHPAD");

  lua_pop(L, 1);
}

static int lGraphicsDrawRect(lua_State *L) {
  auto &rect = udValue<Rect>(L, 1, mtRect);
  auto &color = udValue<Color>(L, 2, mtColor);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawRect(rect, color, angle);
    return 0;
  }
  Graphics::drawRect(rect, color);
  return 0;
}
static int lGraphicsDrawText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &font = udPtr<Font>(L, 2, mtFont);
  auto &pos = udValue<Vec2>(L, 3, mtVec2);
  if (lua_gettop(L) == 3) {
    Graphics::drawText(StringView(text), *font, pos);
    return 0;
  }
  auto &color = udValue<Color>(L, 4, mtColor);
  float size = static_cast<float>(luaL_optnumber(L, 5, 32.0));
  float angle = static_cast<float>(luaL_optnumber(L, 6, 0.0));
  Graphics::drawText(StringView(text), *font, pos, color, size, angle);
  return 0;
}
static int lGraphicsDrawImage(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  auto &rect = udValue<Rect>(L, 2, mtRect);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawImage(*tex, rect, angle);
    return 0;
  }
  Graphics::drawImage(*tex, rect);
  return 0;
}
static int lGraphicsDrawCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, mtTexture);
  Graphics::drawCubemap(*tex);
  return 0;
}
static int lGraphicsDrawModel(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  auto &t = udValue<Transform>(L, 2, mtTransform);
  Graphics::drawModel(*m, t);
  return 0;
}
static int lGraphicsDrawModelInstanced(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, mtModel);
  luaL_checktype(L, 2, LUA_TTABLE);
  Array<Transform> transforms;
  size_t n = lua_objlen(L, 2);
  transforms.reserve(n);
  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 2, static_cast<lua_Integer>(i));
    transforms.push_back(udValue<Transform>(L, -1, mtTransform));
    lua_pop(L, 1);
  }
  Graphics::drawModelInstanced(*m, transforms);
  return 0;
}
static int lGraphicsSetCamera(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, mtCamera);
  Graphics::setCamera(cam);
  return 0;
}
static int lGraphicsGetCamera(lua_State *L) {
  auto &cam = Graphics::getCamera();
  udNewRef<Camera>(L, mtCamera, &cam);
  return 1;
}
static int lGraphicsSetCamera2d(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, mtCamerA2D);
  Graphics::setCamera2D(cam);
  return 0;
}
static int lGraphicsGetCamera2d(lua_State *L) {
  auto &cam = Graphics::getCamera2D();
  udNewRef<Camera2D>(L, mtCamerA2D, &cam);
  return 1;
}
static int lGraphicsGetClearColor(lua_State *L) {
  udNewOwned<Color>(L, mtColor, Graphics::getClearColor());
  return 1;
}
static int lGraphicsSetClearColor(lua_State *L) {
  auto &c = udValue<Color>(L, 1, mtColor);
  Graphics::setClearColor(c);
  return 0;
}
static int lGraphicsSetGraphicsPipeline(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, mtPipeline);
  Graphics::setGraphicsPipeline(*p);
  return 0;
}
static int lGraphicsResetGraphicsPipeline(lua_State *L) {
  (void)L;
  Graphics::resetGraphicsPipeline();
  return 0;
}
static int lGraphicsSetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &t = udPtr<Texture>(L, 2, mtTexture);
  Graphics::setTexture(slot, *t);
  return 0;
}
static int lGraphicsResetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Graphics::resetTexture(slot);
  return 0;
}
static int lGraphicsResetAllTexture(lua_State *L) {
  (void)L;
  Graphics::resetAllTexture();
  return 0;
}
static int lGraphicsSetUniformBuffer(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &b = udValue<Buffer>(L, 2, mtBuffer);
  Graphics::setUniformBuffer(slot, b);
  return 0;
}
static int lGraphicsSetRenderTarget(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, mtRendertexture);
  Graphics::setRenderTarget(*rt);
  return 0;
}
static int lGraphicsFlush(lua_State *L) {
  (void)L;
  Graphics::flush();
  return 0;
}
static int lGraphicsReadbackTexture(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, mtRendertexture);
  auto &out = udPtr<Texture>(L, 2, mtTexture);
  lua_pushboolean(L, Graphics::readbackTexture(*rt, *out));
  return 1;
}
static void registerGraphics(lua_State *L) {
  pushSnNamed(L, "Graphics");
  luaPushcfunction2(L, lGraphicsDrawRect);
  lua_setfield(L, -2, "drawRect");
  luaPushcfunction2(L, lGraphicsDrawText);
  lua_setfield(L, -2, "drawText");
  luaPushcfunction2(L, lGraphicsDrawImage);
  lua_setfield(L, -2, "drawImage");
  luaPushcfunction2(L, lGraphicsDrawCubemap);
  lua_setfield(L, -2, "drawCubemap");
  luaPushcfunction2(L, lGraphicsDrawModel);
  lua_setfield(L, -2, "drawModel");
  luaPushcfunction2(L, lGraphicsDrawModelInstanced);
  lua_setfield(L, -2, "drawModelInstanced");
  luaPushcfunction2(L, lGraphicsSetCamera);
  lua_setfield(L, -2, "setCamera");
  luaPushcfunction2(L, lGraphicsGetCamera);
  lua_setfield(L, -2, "getCamera");
  luaPushcfunction2(L, lGraphicsSetCamera2d);
  lua_setfield(L, -2, "setCamera2d");
  luaPushcfunction2(L, lGraphicsGetCamera2d);
  lua_setfield(L, -2, "getCamera2d");
  luaPushcfunction2(L, lGraphicsGetClearColor);
  lua_setfield(L, -2, "getClearColor");
  luaPushcfunction2(L, lGraphicsSetClearColor);
  lua_setfield(L, -2, "setClearColor");
  luaPushcfunction2(L, lGraphicsSetGraphicsPipeline);
  lua_setfield(L, -2, "setGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsResetGraphicsPipeline);
  lua_setfield(L, -2, "resetGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsSetTexture);
  lua_setfield(L, -2, "setTexture");
  luaPushcfunction2(L, lGraphicsResetTexture);
  lua_setfield(L, -2, "resetTexture");
  luaPushcfunction2(L, lGraphicsResetAllTexture);
  lua_setfield(L, -2, "resetAllTexture");
  luaPushcfunction2(L, lGraphicsSetUniformBuffer);
  lua_setfield(L, -2, "setUniformBuffer");
  luaPushcfunction2(L, lGraphicsSetRenderTarget);
  lua_setfield(L, -2, "setRenderTarget");
  luaPushcfunction2(L, lGraphicsFlush);
  lua_setfield(L, -2, "flush");
  luaPushcfunction2(L, lGraphicsReadbackTexture);
  lua_setfield(L, -2, "readbackTexture");
  lua_pop(L, 1);
}

static int lMouseGetPositionOnScene(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Mouse::getPositionOnScene());
  return 1;
}
static int lMouseGetPosition(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Mouse::getPosition());
  return 1;
}
static int lMouseIsPressed(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isPressed(b));
  return 1;
}
static int lMouseIsReleased(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isReleased(b));
  return 1;
}
static int lMouseIsDown(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isDown(b));
  return 1;
}
static int lMouseSetPosition(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1, mtVec2);
  Mouse::setPosition(p);
  return 0;
}
static int lMouseSetPositionOnScene(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1, mtVec2);
  Mouse::setPositionOnScene(p);
  return 0;
}
static int lMouseGetScrollWheel(lua_State *L) {
  udNewOwned<Vec2>(L, mtVec2, Mouse::getScrollWheel());
  return 1;
}
static int lMouseHideCursor(lua_State *L) {
  bool hide = lua_toboolean(L, 1) != 0;
  Mouse::hideCursor(hide);
  return 0;
}
static int lMouseSetRelative(lua_State *L) {
  bool rel = lua_toboolean(L, 1) != 0;
  Mouse::setRelative(rel);
  return 0;
}
static int lMouseIsRelative(lua_State *L) {
  lua_pushboolean(L, Mouse::isRelative());
  return 1;
}
static void registerMouse(lua_State *L) {
  pushSnNamed(L, "Mouse");
  luaPushcfunction2(L, lMouseSetRelative);
  lua_setfield(L, -2, "setRelative");
  luaPushcfunction2(L, lMouseIsRelative);
  lua_setfield(L, -2, "isRelative");
  luaPushcfunction2(L, lMouseGetPositionOnScene);
  lua_setfield(L, -2, "getPositionOnScene");
  luaPushcfunction2(L, lMouseGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lMouseIsPressed);
  lua_setfield(L, -2, "isPressed");
  luaPushcfunction2(L, lMouseIsReleased);
  lua_setfield(L, -2, "isReleased");
  luaPushcfunction2(L, lMouseIsDown);
  lua_setfield(L, -2, "isDown");
  luaPushcfunction2(L, lMouseSetPosition);
  lua_setfield(L, -2, "setPosition");
  luaPushcfunction2(L, lMouseSetPositionOnScene);
  lua_setfield(L, -2, "setPositionOnScene");
  luaPushcfunction2(L, lMouseGetScrollWheel);
  lua_setfield(L, -2, "getScrollWheel");
  luaPushcfunction2(L, lMouseHideCursor);
  lua_setfield(L, -2, "hideCursor");
  lua_pushinteger(L, static_cast<int>(Mouse::LEFT));
  lua_setfield(L, -2, "LEFT");
  lua_pushinteger(L, static_cast<int>(Mouse::RIGHT));
  lua_setfield(L, -2, "RIGHT");
  lua_pushinteger(L, static_cast<int>(Mouse::MIDDLE));
  lua_setfield(L, -2, "MIDDLE");
  lua_pushinteger(L, static_cast<int>(Mouse::X1));
  lua_setfield(L, -2, "X1");
  lua_pushinteger(L, static_cast<int>(Mouse::X2));
  lua_setfield(L, -2, "X2");
  lua_pop(L, 1);
}

static int lFilesystemEnumerateDirectory(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto list = Filesystem::enumerateDirectory(StringView(path));
  lua_newtable(L);
  for (size_t i = 0; i < list.size(); ++i) {
    lua_pushlstring(L, list[i].data(), list[i].size());
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
  return 1;
}
static void registerFilesystem(lua_State *L) {
  pushSnNamed(L, "Filesystem");
  luaPushcfunction2(L, lFilesystemEnumerateDirectory);
  lua_setfield(L, -2, "enumerateDirectory");
  lua_pop(L, 1);
}

static int lScriptLoad(lua_State *L) {
  int n = lua_gettop(L);
  const char *filePath = luaL_checkstring(L, 1);
  if (n >= 2) {
    const char *baseDirPath = luaL_checkstring(L, 2);
    Script::load(StringView(filePath), StringView(baseDirPath));
    return 0;
  }
  Script::load(StringView(filePath), ".");
  return 0;
}
static void registerScript(lua_State *L) {
  pushSnNamed(L, "Script");
  luaPushcfunction2(L, lScriptLoad);
  lua_setfield(L, -2, "load");
  lua_pop(L, 1);
}

static int lLogInfo(lua_State *L) {
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, 1);
  if (luaPCallLogged(L, 1, 1) != LUA_OK) {
    return 0;
  }
  const char *s = lua_tostring(L, -1);
  Log::info(s);
  lua_pop(L, 1);
  return 0;
}
static int lLogVerbose(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::verbose(s);
  return 0;
}
static int lLogError(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::error(s);
  return 0;
}
static int lLogWarn(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::warn(s);
  return 0;
}
static int lLogCritical(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Log::critical(s);
  return 0;
}
static void registerLog(lua_State *L) {
  pushSnNamed(L, "Log");
  luaPushcfunction2(L, lLogVerbose);
  lua_setfield(L, -2, "verbose");
  luaPushcfunction2(L, lLogInfo);
  lua_setfield(L, -2, "info");
  luaPushcfunction2(L, lLogError);
  lua_setfield(L, -2, "error");
  luaPushcfunction2(L, lLogWarn);
  lua_setfield(L, -2, "warn");
  luaPushcfunction2(L, lLogCritical);
  lua_setfield(L, -2, "critical");
  lua_pop(L, 1);
}

static int lPeriodicSin01(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::sineWave(period, t));
  return 1;
}
static int lPeriodicCos01(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::cosWave(period, t));
  return 1;
}
static void registerPeriodic(lua_State *L) {
  pushSnNamed(L, "Periodic");
  luaPushcfunction2(L, lPeriodicSin01);
  lua_setfield(L, -2, "sin0_1");
  luaPushcfunction2(L, lPeriodicCos01);
  lua_setfield(L, -2, "cos0_1");
  lua_pop(L, 1);
}

static int lTimeSeconds(lua_State *L) {
  lua_pushnumber(L, Time::seconds());
  return 1;
}
static int lTimeMilli(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(Time::milli()));
  return 1;
}
static int lTimeDelta(lua_State *L) {
  lua_pushnumber(L, Time::deltaTime());
  return 1;
}
static void registerTime(lua_State *L) {
  pushSnNamed(L, "Time");
  luaPushcfunction2(L, lTimeSeconds);
  lua_setfield(L, -2, "seconds");
  luaPushcfunction2(L, lTimeMilli);
  lua_setfield(L, -2, "milli");
  luaPushcfunction2(L, lTimeDelta);
  lua_setfield(L, -2, "delta");
  lua_pop(L, 1);
}

static int lImGuiBegin(lua_State *L) {
  int n = lua_gettop(L);
  const char *name = luaL_checkstring(L, 1);
  if (n >= 2) {
    int flags = static_cast<int>(luaL_checkinteger(L, 2));
    ImGui::Begin(name, nullptr, flags);
    return 0;
  }
  lua_pushboolean(L, ImGui::Begin(name));
  return 1;
}
static int lImGuiEnd(lua_State *L) {
  (void)L;
  ImGui::End();
  return 0;
}
static int lImGuiButton(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_pushboolean(L, ImGui::Button(name));
  return 1;
}
static int lImGuiText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  ImGui::Text("%s", text);
  return 0;
}
static int lImGuiSetNextWindowPos(lua_State *L) {
  auto &pos = udValue<Vec2>(L, 1, mtVec2);
  ImGui::SetNextWindowPos({pos.x, pos.y});
  return 0;
}
static int lImGuiSetNextWindowSize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1, mtVec2);
  ImGui::SetNextWindowSize({size.x, size.y});
  return 0;
}
static void registerImGui(lua_State *L) {
  pushSnNamed(L, "ImGui");
  luaPushcfunction2(L, lImGuiBegin);
  lua_setfield(L, -2, "Begin");
  luaPushcfunction2(L, lImGuiEnd);
  lua_setfield(L, -2, "End");
  luaPushcfunction2(L, lImGuiButton);
  lua_setfield(L, -2, "button");
  luaPushcfunction2(L, lImGuiText);
  lua_setfield(L, -2, "text");
  luaPushcfunction2(L, lImGuiSetNextWindowPos);
  lua_setfield(L, -2, "setNextWindowPos");
  luaPushcfunction2(L, lImGuiSetNextWindowSize);
  lua_setfield(L, -2, "setNextWindowSize");

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "WindowFlags");
  lua_pushinteger(L, ImGuiWindowFlags_None);
  lua_setfield(L, -2, "None");
  lua_pushinteger(L, ImGuiWindowFlags_NoTitleBar);
  lua_setfield(L, -2, "NoTitleBar");
  lua_pushinteger(L, ImGuiWindowFlags_NoResize);
  lua_setfield(L, -2, "NoResize");
  lua_pushinteger(L, ImGuiWindowFlags_NoMove);
  lua_setfield(L, -2, "NoMove");
  lua_pushinteger(L, ImGuiWindowFlags_NoScrollbar);
  lua_setfield(L, -2, "NoScrollBar");
  lua_pushinteger(L, ImGuiWindowFlags_NoScrollWithMouse);
  lua_setfield(L, -2, "NoScrollWithMouse");
  lua_pushinteger(L, ImGuiWindowFlags_NoCollapse);
  lua_setfield(L, -2, "NoCollapse");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysAutoResize);
  lua_setfield(L, -2, "AlwaysAutoResize");
  lua_pushinteger(L, ImGuiWindowFlags_NoBackground);
  lua_setfield(L, -2, "NoBackground");
  lua_pushinteger(L, ImGuiWindowFlags_NoSavedSettings);
  lua_setfield(L, -2, "NoSavedSettings");
  lua_pushinteger(L, ImGuiWindowFlags_NoMouseInputs);
  lua_setfield(L, -2, "NoMouseInputs");
  lua_pushinteger(L, ImGuiWindowFlags_MenuBar);
  lua_setfield(L, -2, "MenuBar");
  lua_pushinteger(L, ImGuiWindowFlags_HorizontalScrollbar);
  lua_setfield(L, -2, "HorizontalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_NoFocusOnAppearing);
  lua_setfield(L, -2, "NoFocusOnAppearing");
  lua_pushinteger(L, ImGuiWindowFlags_NoBringToFrontOnFocus);
  lua_setfield(L, -2, "NoBringToFrontOnFocus");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysVerticalScrollbar);
  lua_setfield(L, -2, "AlwaysVerticalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
  lua_setfield(L, -2, "AlwaysHorizontalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_NoNavInputs);
  lua_setfield(L, -2, "NoNavInputs");
  lua_pushinteger(L, ImGuiWindowFlags_NoNavFocus);
  lua_setfield(L, -2, "NoNavFocus");
  lua_pushinteger(L, ImGuiWindowFlags_UnsavedDocument);
  lua_setfield(L, -2, "UnsavedDocument");
  lua_pushinteger(L, ImGuiWindowFlags_NoNav);
  lua_setfield(L, -2, "NoNav");
  lua_pushinteger(L, ImGuiWindowFlags_NoDecoration);
  lua_setfield(L, -2, "NoDecoration");
  lua_pushinteger(L, ImGuiWindowFlags_NoInputs);
  lua_setfield(L, -2, "NoInputs");
  lua_pop(L, 2); // WindowFlags + ImGui
}

static int luaLoadSource(lua_State *L, const String &source,
                         const String &chunkname) {
#ifdef SINEN_USE_LUAU
  std::string bytecode =
      Luau::compile(std::string(source.data(), source.size()));
  int status =
      luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
  if (status == LUA_OK) {
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[luau load error] %s", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
#else
  return luaL_loadbuffer(L, source.data(), source.size(), chunkname.c_str());
#endif
}
static int lImport(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  String filename = String(name) + ".lua";
  String source = AssetIO::openAsString(filename);
  if (source.empty()) {
    lua_pushnil(L);
    return 1;
  }
  String chunkname = "@" + AssetIO::getFilePath(filename);
  if (luaLoadSource(L, source, chunkname) != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    LogF::error("[lua load error] {}", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
    lua_pushnil(L);
    return 1;
  }
  int topBefore = lua_gettop(L) - 1;
  if (luaPCallLogged(L, 0, LUA_MULTRET) != LUA_OK) {
    lua_pushnil(L);
    return 1;
  }
  int nret = lua_gettop(L) - topBefore;
  if (nret <= 0) {
    lua_pushnil(L);
    return 1;
  }
  return nret;
}

static void registerAll(lua_State *L) {

#ifdef SINEN_USE_LUAU
  lua_pushcfunction2(L, l_import);
  lua_setglobal(L, "require");
#endif

  luaPushcfunction2(L, lImport);
  lua_setglobal(L, "import");

  registerVec2(L);
  registerVec3(L);
  registerColor(L);
  registerAABB(L);
  registerRay(L);
  registerTimer(L);
  registerCollider(L);
  registerCamera(L);
  registerCamera2D(L);
  registerBuffer(L);
  registerPivot(L);
  registerRect(L);
  registerTransform(L);
  registerGrid(L);
  registerBFSGrid(L);

  registerFont(L);
  registerTexture(L);
  registerRenderTexture(L);
  registerSound(L);
  registerShader(L);
  registerPipeline(L);
  registerModel(L);
  registerTextureKey(L);

  registerArguments(L);
  registerRandom(L);
  registerWindow(L);
  registerPhysics(L);
  registerShaderStage(L);
  registerBuiltinShader(L);
  registerBuiltinPipeline(L);
  registerEvent(L);

  registerGraphics(L);
  registerMouse(L);
  registerKeyboard(L);
  registerGamepad(L);
  registerFilesystem(L);
  registerScript(L);
  registerLog(L);
  registerImGui(L);
  registerPeriodic(L);
  registerTime(L);
}

} // namespace

bool Script::initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  // bindings are implemented using Lua C API in this file
  gLua = lua_newstate(alloc, nullptr);
  if (!gLua) {
    Log::error("lua_newstate failed");
    return false;
  }
  luaL_openlibs(gLua);
#ifdef SINEN_USE_LUAU
  if (auto *cb = lua_callbacks(gLua)) {
    cb->panic = [](lua_State *L, int errcode) {
      const char *msg = lua_tostring(L, -1);
      Log::critical("[luau panic %d] %s", errcode,
                    msg ? msg : "(unknown error)");
    };
  }
#else
  lua_atpanic(gLua, [](lua_State *L) -> int {
    const char *msg = lua_tostring(L, -1);
    LogF::critical("[lua panic] {}", msg ? msg : "(unknown error)");
    return 0;
  });
#endif

  lua_newtable(gLua);
  lua_setglobal(gLua, "sn");

  registerAll(gLua);
#endif
  return true;
}
void Script::shutdown() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }
  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }
  lua_gc(gLua, LUA_GCCOLLECT, 0);
  lua_close(gLua);
  gLua = nullptr;
#endif // SINEN_NO_USE_SCRIPT
}

static const char *nothingSceneLua = R"(
local font = sn.Font.new()
font:load(32)
function draw()
    sn.Graphics.drawText("NO DATA", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32, 0.0)
end
)";
void Script::runScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }

  auto logPCallError = [](lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    LogF::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
  };

  String source;
  source = AssetIO::openAsString(String(sceneName) + ".lua");
  if (source.empty()) {
    source = nothingSceneLua;
  }

  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }

  String filename = String(sceneName) + ".lua";
  String chunkname = "@" + AssetIO::getFilePath(filename);
  if (luaLoadSource(gLua, source, chunkname) != LUA_OK) {
    logPCallError(gLua);
    return;
  }
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    logPCallError(gLua);
    return;
  }

  lua_getglobal(gLua, "setup");
  int funcIndex = LUA_REGISTRYINDEX;
#ifdef SINEN_USE_LUAU
  funcIndex = -1;
#endif
  if (lua_isfunction(gLua, -1)) {
    gSetupRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "update");
  if (lua_isfunction(gLua, -1)) {
    gUpdateRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "draw");
  if (lua_isfunction(gLua, -1)) {
    gDrawRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }

  if (gSetupRef != LUA_NOREF) {
    lua_rawgeti(gLua, LUA_REGISTRYINDEX, gSetupRef);
    if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
      logPCallError(gLua);
    }
  }
#endif
}

void Script::updateScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua || gUpdateRef == LUA_NOREF) {
    return;
  }
  lua_rawgeti(gLua, LUA_REGISTRYINDEX, gUpdateRef);
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    const char *msg = lua_tostring(gLua, -1);
    LogF::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(gLua, 1);
  }
#endif
}

void Script::drawScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua || gDrawRef == LUA_NOREF) {
    return;
  }
  lua_rawgeti(gLua, LUA_REGISTRYINDEX, gDrawRef);
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    const char *msg = lua_tostring(gLua, -1);
    LogF::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(gLua, 1);
  }
#endif
}

} // namespace sinen
