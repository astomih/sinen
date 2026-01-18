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
#include <core/logger/logger.hpp>
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

#include <lua.hpp>

#include <imgui.h>

#include <cstddef>
#include <cstring>
#include <format>
#include <utility>

namespace sinen {
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

template <class T> static T &udValue(lua_State *L, int idx, const char *mt) {
  auto *ud = static_cast<UdBox<T> *>(luaL_checkudata(L, idx, mt));
  return *ud->ptr;
}
template <class T>
static T *udValueOrNull(lua_State *L, int idx, const char *mt) {
  auto *ud = static_cast<UdBox<T> *>(luaL_testudata(L, idx, mt));
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
  Logger::error("[lua error] %s", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return LUA_ERRRUN;
}

// metatable names
static constexpr const char *MT_VEC2 = "sn.Vec2";
static constexpr const char *MT_VEC3 = "sn.Vec3";
static constexpr const char *MT_COLOR = "sn.Color";
static constexpr const char *MT_RECT = "sn.Rect";
static constexpr const char *MT_TRANSFORM = "sn.Transform";
static constexpr const char *MT_FONT = "sn.Font";
static constexpr const char *MT_TEXTURE = "sn.Texture";
static constexpr const char *MT_MODEL = "sn.Model";
static constexpr const char *MT_BUFFER = "sn.Buffer";
static constexpr const char *MT_RENDERTEXTURE = "sn.RenderTexture";
static constexpr const char *MT_SOUND = "sn.Sound";
static constexpr const char *MT_CAMERA = "sn.Camera";
static constexpr const char *MT_CAMERA2D = "sn.Camera2D";
static constexpr const char *MT_AABB = "sn.AABB";
static constexpr const char *MT_TIMER = "sn.Timer";
static constexpr const char *MT_COLLIDER = "sn.Collider";
static constexpr const char *MT_SHADER = "sn.Shader";
static constexpr const char *MT_PIPELINE = "sn.GraphicsPipeline";
static constexpr const char *MT_GRID = "sn.Grid";
static constexpr const char *MT_BFSGRID = "sn.BFSGrid";
static constexpr const char *MT_MOUSE = "sn.Mouse";

// -----------------
// Vec2
// -----------------
static int l_Vec2_new(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec2>(L, MT_VEC2, Vec2(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec2>(L, MT_VEC2, Vec2(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Vec2>(L, MT_VEC2, Vec2(x, y));
  return 1;
}
static int l_Vec2_index(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, MT_VEC2);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, v.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, v.y);
    return 1;
  }
  luaL_getmetatable(L, MT_VEC2);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lVec2Newindex(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, MT_VEC2);
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
  return luaL_error(L, "sn.Vec2: invalid field '%s'", k);
}
static int l_Vec2_add(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, MT_VEC2);
  auto &b = udValue<Vec2>(L, 2, MT_VEC2);
  udNewOwned<Vec2>(L, MT_VEC2, a + b);
  return 1;
}
static int l_Vec2_sub(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, MT_VEC2);
  auto &b = udValue<Vec2>(L, 2, MT_VEC2);
  udNewOwned<Vec2>(L, MT_VEC2, a - b);
  return 1;
}
static int l_Vec2_mul(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, MT_VEC2);
  auto &b = udValue<Vec2>(L, 2, MT_VEC2);
  udNewOwned<Vec2>(L, MT_VEC2, a * b);
  return 1;
}
static int l_Vec2_div(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1, MT_VEC2);
  auto &b = udValue<Vec2>(L, 2, MT_VEC2);
  udNewOwned<Vec2>(L, MT_VEC2, a / b);
  return 1;
}
static int l_Vec2_tostring(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, MT_VEC2);
  String s = vec2Str(v);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int l_Vec2_copy(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, MT_VEC2);
  udNewOwned<Vec2>(L, MT_VEC2, v);
  return 1;
}
static int l_Vec2_length(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1, MT_VEC2);
  lua_pushnumber(L, v.length());
  return 1;
}
static void registerVec2(lua_State *L) {
  luaL_newmetatable(L, MT_VEC2);
  lua_pushcfunction(L, udGc<Vec2>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_Vec2_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, lVec2Newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, l_Vec2_add);
  lua_setfield(L, -2, "__add");
  lua_pushcfunction(L, l_Vec2_sub);
  lua_setfield(L, -2, "__sub");
  lua_pushcfunction(L, l_Vec2_mul);
  lua_setfield(L, -2, "__mul");
  lua_pushcfunction(L, l_Vec2_div);
  lua_setfield(L, -2, "__div");
  lua_pushcfunction(L, l_Vec2_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pushcfunction(L, l_Vec2_copy);
  lua_setfield(L, -2, "copy");
  lua_pushcfunction(L, l_Vec2_length);
  lua_setfield(L, -2, "length");
  lua_pop(L, 1);

  pushSnNamed(L, "Vec2");
  lua_pushcfunction(L, l_Vec2_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Vec3
// -----------------
static int l_Vec3_new(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec3>(L, MT_VEC3, Vec3(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec3>(L, MT_VEC3, Vec3(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  float z = static_cast<float>(luaL_checknumber(L, 3));
  udNewOwned<Vec3>(L, MT_VEC3, Vec3(x, y, z));
  return 1;
}
static int l_Vec3_index(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, MT_VEC3);
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
  luaL_getmetatable(L, MT_VEC3);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int l_Vec3_newindex(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, MT_VEC3);
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
  return luaL_error(L, "sn.Vec3: invalid field '%s'", k);
}
static int l_Vec3_add(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, MT_VEC3);
  auto &b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, a + b);
  return 1;
}
static int l_Vec3_sub(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, MT_VEC3);
  auto &b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, a - b);
  return 1;
}
static int l_Vec3_mul(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, MT_VEC3);
  auto &b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, a * b);
  return 1;
}
static int l_Vec3_div(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1, MT_VEC3);
  auto &b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, a / b);
  return 1;
}
static int l_Vec3_tostring(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, MT_VEC3);
  String s = vec3Str(v);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int l_Vec3_copy(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, v);
  return 1;
}
static int l_Vec3_length(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1, MT_VEC3);
  lua_pushnumber(L, v.length());
  return 1;
}
static int l_Vec3_normalize(lua_State *L) {
  auto v = udValue<Vec3>(L, 1, MT_VEC3);
  v.normalize();
  udNewOwned<Vec3>(L, MT_VEC3, v);
  return 1;
}
static int lVec3Dot(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, MT_VEC3);
  auto b = udValue<Vec3>(L, 2, MT_VEC3);
  lua_pushnumber(L, Vec3::dot(a, b));
  return 1;
}
static int lVec3Cross(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, MT_VEC3);
  auto b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, Vec3::cross(a, b));
  return 1;
}
static int lVec3Lerp(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, MT_VEC3);
  auto b = udValue<Vec3>(L, 2, MT_VEC3);
  float value = luaL_checknumber(L, 3);
  udNewOwned<Vec3>(L, MT_VEC3, Vec3::lerp(a, b, value));
  return 1;
}
static int lVec3Reflect(lua_State *L) {
  auto a = udValue<Vec3>(L, 1, MT_VEC3);
  auto b = udValue<Vec3>(L, 2, MT_VEC3);
  udNewOwned<Vec3>(L, MT_VEC3, Vec3::reflect(a, b));
  return 1;
}
static void registerVec3(lua_State *L) {
  luaL_newmetatable(L, MT_VEC3);
  lua_pushcfunction(L, udGc<Vec3>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_Vec3_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Vec3_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, l_Vec3_add);
  lua_setfield(L, -2, "__add");
  lua_pushcfunction(L, l_Vec3_sub);
  lua_setfield(L, -2, "__sub");
  lua_pushcfunction(L, l_Vec3_mul);
  lua_setfield(L, -2, "__mul");
  lua_pushcfunction(L, l_Vec3_div);
  lua_setfield(L, -2, "__div");
  lua_pushcfunction(L, l_Vec3_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pushcfunction(L, l_Vec3_copy);
  lua_setfield(L, -2, "copy");
  lua_pushcfunction(L, l_Vec3_length);
  lua_setfield(L, -2, "length");
  lua_pushcfunction(L, l_Vec3_normalize);
  lua_setfield(L, -2, "normalize");
  lua_pushcfunction(L, lVec3Dot);
  lua_setfield(L, -2, "dot");
  lua_pushcfunction(L, lVec3Cross);
  lua_setfield(L, -2, "cross");
  lua_pushcfunction(L, lVec3Lerp);
  lua_setfield(L, -2, "lerp");
  lua_pushcfunction(L, lVec3Reflect);
  lua_setfield(L, -2, "reflect");
  lua_pop(L, 1);

  pushSnNamed(L, "Vec3");
  lua_pushcfunction(L, l_Vec3_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Color
// -----------------
static int l_Color_new(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Color>(L, MT_COLOR, Color(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Color>(L, MT_COLOR, Color(v));
    return 1;
  }
  if (n == 2) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    float a = static_cast<float>(luaL_checknumber(L, 2));
    udNewOwned<Color>(L, MT_COLOR, Color(v, a));
    return 1;
  }
  float r = static_cast<float>(luaL_checknumber(L, 1));
  float g = static_cast<float>(luaL_checknumber(L, 2));
  float b = static_cast<float>(luaL_checknumber(L, 3));
  float a = static_cast<float>(luaL_optnumber(L, 4, 1.0));
  udNewOwned<Color>(L, MT_COLOR, Color(r, g, b, a));
  return 1;
}
static int l_Color_index(lua_State *L) {
  auto &c = udValue<Color>(L, 1, MT_COLOR);
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
  luaL_getmetatable(L, MT_COLOR);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int l_Color_newindex(lua_State *L) {
  auto &c = udValue<Color>(L, 1, MT_COLOR);
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
  return luaL_error(L, "sn.Color: invalid field '%s'", k);
}
static int l_Color_tostring(lua_State *L) {
  auto &c = udValue<Color>(L, 1, MT_COLOR);
  String s = colorStr(c);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerColor(lua_State *L) {
  luaL_newmetatable(L, MT_COLOR);
  lua_pushcfunction(L, udGc<Color>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_Color_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Color_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, l_Color_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Color");
  lua_pushcfunction(L, l_Color_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Camera / Camera2D
// -----------------
static int l_Camera_new(lua_State *L) {
  udNewOwned<Camera>(L, MT_CAMERA, Camera{});
  return 1;
}
static int l_Camera_lookat(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  auto &pos = udValue<Vec3>(L, 2, MT_VEC3);
  auto &target = udValue<Vec3>(L, 3, MT_VEC3);
  auto &up = udValue<Vec3>(L, 4, MT_VEC3);
  cam.lookat(pos, target, up);
  return 0;
}
static int l_Camera_perspective(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  float fov = static_cast<float>(luaL_checknumber(L, 2));
  float aspect = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.perspective(fov, aspect, nearZ, farZ);
  return 0;
}
static int l_Camera_orthographic(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  float w = static_cast<float>(luaL_checknumber(L, 2));
  float h = static_cast<float>(luaL_checknumber(L, 3));
  float nearZ = static_cast<float>(luaL_checknumber(L, 4));
  float farZ = static_cast<float>(luaL_checknumber(L, 5));
  cam.orthographic(w, h, nearZ, farZ);
  return 0;
}
static int l_Camera_getPosition(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  udNewRef<Vec3>(L, MT_VEC3, &cam.getPosition());
  return 1;
}
static int l_Camera_getTarget(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  udNewOwned<Vec3>(L, MT_VEC3, cam.getTarget());
  return 1;
}
static int l_Camera_getUp(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  udNewOwned<Vec3>(L, MT_VEC3, cam.getUp());
  return 1;
}
static int l_Camera_isAABBInFrustum(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  auto &aabb = udValue<AABB>(L, 2, MT_AABB);
  lua_pushboolean(L, cam.isAABBInFrustum(aabb));
  return 1;
}
static void registerCamera(lua_State *L) {
  luaL_newmetatable(L, MT_CAMERA);
  lua_pushcfunction(L, udGc<Camera>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Camera_lookat);
  lua_setfield(L, -2, "lookat");
  lua_pushcfunction(L, l_Camera_perspective);
  lua_setfield(L, -2, "perspective");
  lua_pushcfunction(L, l_Camera_orthographic);
  lua_setfield(L, -2, "orthographic");
  lua_pushcfunction(L, l_Camera_getPosition);
  lua_setfield(L, -2, "getPosition");
  lua_pushcfunction(L, l_Camera_getTarget);
  lua_setfield(L, -2, "getTarget");
  lua_pushcfunction(L, l_Camera_getUp);
  lua_setfield(L, -2, "getUp");
  lua_pushcfunction(L, l_Camera_isAABBInFrustum);
  lua_setfield(L, -2, "isAABBInFrustum");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera");
  lua_pushcfunction(L, l_Camera_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Camera2D_new(lua_State *L) {
  udNewOwned<Camera2D>(L, MT_CAMERA2D, Camera2D{});
  return 1;
}
static int l_Camera2D_size(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  udNewOwned<Vec2>(L, MT_VEC2, cam.size());
  return 1;
}
static int l_Camera2D_half(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  udNewOwned<Vec2>(L, MT_VEC2, cam.half());
  return 1;
}
static int l_Camera2D_resize(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  auto &size = udValue<Vec2>(L, 2, MT_VEC2);
  cam.resize(size);
  return 0;
}
static int l_Camera2D_windowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  udNewOwned<Vec2>(L, MT_VEC2, cam.windowRatio());
  return 1;
}
static int l_Camera2D_invWindowRatio(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  udNewOwned<Vec2>(L, MT_VEC2, cam.invWindowRatio());
  return 1;
}
static void registerCamera2D(lua_State *L) {
  luaL_newmetatable(L, MT_CAMERA2D);
  lua_pushcfunction(L, udGc<Camera2D>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Camera2D_size);
  lua_setfield(L, -2, "size");
  lua_pushcfunction(L, l_Camera2D_half);
  lua_setfield(L, -2, "half");
  lua_pushcfunction(L, l_Camera2D_resize);
  lua_setfield(L, -2, "resize");
  lua_pushcfunction(L, l_Camera2D_windowRatio);
  lua_setfield(L, -2, "windowRatio");
  lua_pushcfunction(L, l_Camera2D_invWindowRatio);
  lua_setfield(L, -2, "invWindowRatio");
  lua_pop(L, 1);

  pushSnNamed(L, "Camera2D");
  lua_pushcfunction(L, l_Camera2D_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// AABB / Timer / Collider
// -----------------
static int l_AABB_new(lua_State *L) {
  udNewOwned<AABB>(L, MT_AABB, AABB{});
  return 1;
}
static int l_AABB_index(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, MT_AABB);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "min") == 0) {
    udNewRef<Vec3>(L, MT_VEC3, &aabb.min);
    return 1;
  }
  if (std::strcmp(k, "max") == 0) {
    udNewRef<Vec3>(L, MT_VEC3, &aabb.max);
    return 1;
  }
  luaL_getmetatable(L, MT_AABB);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int l_AABB_newindex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, MT_AABB);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3, MT_VEC3);
  if (std::strcmp(k, "min") == 0) {
    aabb.min = v;
    return 0;
  }
  if (std::strcmp(k, "max") == 0) {
    aabb.max = v;
    return 0;
  }
  return luaL_error(L, "sn.AABB: invalid field '%s'", k);
}
static int l_AABB_updateWorld(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1, MT_AABB);
  auto &p = udValue<Vec3>(L, 2, MT_VEC3);
  auto &scale = udValue<Vec3>(L, 3, MT_VEC3);
  auto &local = udValue<AABB>(L, 4, MT_AABB);
  aabb.updateWorld(p, scale, local);
  return 0;
}
static int l_AABB_intersectsAABB(lua_State *L) {
  auto &a = udValue<AABB>(L, 1, MT_AABB);
  auto &b = udValue<AABB>(L, 2, MT_AABB);
  lua_pushboolean(L, a.intersectsAABB(b));
  return 1;
}
static void registerAABB(lua_State *L) {
  luaL_newmetatable(L, MT_AABB);
  lua_pushcfunction(L, udGc<AABB>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_AABB_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_AABB_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, l_AABB_updateWorld);
  lua_setfield(L, -2, "updateWorld");
  lua_pushcfunction(L, l_AABB_intersectsAABB);
  lua_setfield(L, -2, "intersectsAABB");
  lua_pop(L, 1);

  pushSnNamed(L, "AABB");
  lua_pushcfunction(L, l_AABB_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Timer_new(lua_State *L) {
  if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
    float t = static_cast<float>(lua_tonumber(L, 1));
    udNewOwned<Timer>(L, MT_TIMER, Timer(t));
    return 1;
  }
  udNewOwned<Timer>(L, MT_TIMER, Timer());
  return 1;
}
static int l_Timer_start(lua_State *L) {
  udValue<Timer>(L, 1, MT_TIMER).start();
  return 0;
}
static int l_Timer_stop(lua_State *L) {
  udValue<Timer>(L, 1, MT_TIMER).stop();
  return 0;
}
static int l_Timer_isStarted(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1, MT_TIMER).isStarted());
  return 1;
}
static int l_Timer_setTime(lua_State *L) {
  float ms = static_cast<float>(luaL_checknumber(L, 2));
  udValue<Timer>(L, 1, MT_TIMER).setTime(ms);
  return 0;
}
static int l_Timer_check(lua_State *L) {
  lua_pushboolean(L, udValue<Timer>(L, 1, MT_TIMER).check());
  return 1;
}
static void registerTimer(lua_State *L) {
  luaL_newmetatable(L, MT_TIMER);
  lua_pushcfunction(L, udGc<Timer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Timer_start);
  lua_setfield(L, -2, "start");
  lua_pushcfunction(L, l_Timer_stop);
  lua_setfield(L, -2, "stop");
  lua_pushcfunction(L, l_Timer_isStarted);
  lua_setfield(L, -2, "isStarted");
  lua_pushcfunction(L, l_Timer_setTime);
  lua_setfield(L, -2, "setTime");
  lua_pushcfunction(L, l_Timer_check);
  lua_setfield(L, -2, "check");
  lua_pop(L, 1);

  pushSnNamed(L, "Timer");
  lua_pushcfunction(L, l_Timer_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Collider_new(lua_State *L) {
  udNewOwned<Collider>(L, MT_COLLIDER, Collider{});
  return 1;
}
static int l_Collider_getPosition(lua_State *L) {
  udNewOwned<Vec3>(L, MT_VEC3,
                   udValue<Collider>(L, 1, MT_COLLIDER).getPosition());
  return 1;
}
static int l_Collider_getVelocity(lua_State *L) {
  udNewOwned<Vec3>(L, MT_VEC3,
                   udValue<Collider>(L, 1, MT_COLLIDER).getVelocity());
  return 1;
}
static int l_Collider_setLinearVelocity(lua_State *L) {
  auto &c = udValue<Collider>(L, 1, MT_COLLIDER);
  auto &v = udValue<Vec3>(L, 2, MT_VEC3);
  c.setLinearVelocity(v);
  return 0;
}
static void registerCollider(lua_State *L) {
  luaL_newmetatable(L, MT_COLLIDER);
  lua_pushcfunction(L, udGc<Collider>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Collider_getPosition);
  lua_setfield(L, -2, "getPosition");
  lua_pushcfunction(L, l_Collider_getVelocity);
  lua_setfield(L, -2, "getVelocity");
  lua_pushcfunction(L, l_Collider_setLinearVelocity);
  lua_setfield(L, -2, "setLinearVelocity");
  lua_pop(L, 1);

  pushSnNamed(L, "Collider");
  lua_pushcfunction(L, l_Collider_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Grid / BFSGrid
// -----------------
static int l_Grid_new(lua_State *L) {
  float w = static_cast<float>(luaL_checknumber(L, 1));
  float h = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Grid>(L, MT_GRID,
                   Grid(static_cast<size_t>(w), static_cast<size_t>(h)));
  return 1;
}
static int l_Grid_at(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  lua_pushnumber(L, g.at(x, y));
  return 1;
}
static int l_Grid_set(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  int x = static_cast<int>(luaL_checkinteger(L, 2));
  int y = static_cast<int>(luaL_checkinteger(L, 3));
  float v = static_cast<float>(luaL_checknumber(L, 4));
  g.at(x, y) = v;
  return 0;
}
static int l_Grid_width(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<Grid>(L, 1, MT_GRID).width()));
  return 1;
}
static int l_Grid_height(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<Grid>(L, 1, MT_GRID).height()));
  return 1;
}
static int l_Grid_size(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<Grid>(L, 1, MT_GRID).size()));
  return 1;
}
static int l_Grid_clear(lua_State *L) {
  udValue<Grid>(L, 1, MT_GRID).clear();
  return 0;
}
static int l_Grid_resize(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  size_t w = static_cast<size_t>(luaL_checkinteger(L, 2));
  size_t h = static_cast<size_t>(luaL_checkinteger(L, 3));
  g.resize(w, h);
  return 0;
}
static int l_Grid_fill(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  g.fill(v);
  return 0;
}
static int l_Grid_fillRect(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  auto &r = udValue<Rect>(L, 2, MT_RECT);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.fillRect(r, v);
  return 0;
}
static int l_Grid_setRow(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setRow(idx, v);
  return 0;
}
static int l_Grid_setColumn(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  int idx = static_cast<int>(luaL_checkinteger(L, 2));
  float v = static_cast<float>(luaL_checknumber(L, 3));
  g.setColumn(idx, v);
  return 0;
}
static void registerGrid(lua_State *L) {
  luaL_newmetatable(L, MT_GRID);
  lua_pushcfunction(L, udGc<Grid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Grid_at);
  lua_setfield(L, -2, "at");
  lua_pushcfunction(L, l_Grid_set);
  lua_setfield(L, -2, "set");
  lua_pushcfunction(L, l_Grid_width);
  lua_setfield(L, -2, "width");
  lua_pushcfunction(L, l_Grid_height);
  lua_setfield(L, -2, "height");
  lua_pushcfunction(L, l_Grid_size);
  lua_setfield(L, -2, "size");
  lua_pushcfunction(L, l_Grid_clear);
  lua_setfield(L, -2, "clear");
  lua_pushcfunction(L, l_Grid_resize);
  lua_setfield(L, -2, "resize");
  lua_pushcfunction(L, l_Grid_fill);
  lua_setfield(L, -2, "fill");
  lua_pushcfunction(L, l_Grid_fillRect);
  lua_setfield(L, -2, "fillRect");
  lua_pushcfunction(L, l_Grid_setRow);
  lua_setfield(L, -2, "setRow");
  lua_pushcfunction(L, l_Grid_setColumn);
  lua_setfield(L, -2, "setColumn");
  lua_pop(L, 1);

  pushSnNamed(L, "Grid");
  lua_pushcfunction(L, l_Grid_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_BFSGrid_new(lua_State *L) {
  auto &g = udValue<Grid>(L, 1, MT_GRID);
  udNewOwned<BFSGrid>(L, MT_BFSGRID, BFSGrid(g));
  return 1;
}
static int l_BFSGrid_width(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1, MT_BFSGRID).width()));
  return 1;
}
static int l_BFSGrid_height(lua_State *L) {
  lua_pushinteger(
      L, static_cast<lua_Integer>(udValue<BFSGrid>(L, 1, MT_BFSGRID).height()));
  return 1;
}
static int l_BFSGrid_findPath(lua_State *L) {
  auto &b = udValue<BFSGrid>(L, 1, MT_BFSGRID);
  auto &start = udValue<Vec2>(L, 2, MT_VEC2);
  auto &end = udValue<Vec2>(L, 3, MT_VEC2);
  lua_pushboolean(L, b.findPath(start, end));
  return 1;
}
static int l_BFSGrid_trace(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, udValue<BFSGrid>(L, 1, MT_BFSGRID).trace());
  return 1;
}
static int l_BFSGrid_traceable(lua_State *L) {
  lua_pushboolean(L, udValue<BFSGrid>(L, 1, MT_BFSGRID).traceable());
  return 1;
}
static int l_BFSGrid_reset(lua_State *L) {
  udValue<BFSGrid>(L, 1, MT_BFSGRID).reset();
  return 0;
}
static void registerBFSGrid(lua_State *L) {
  luaL_newmetatable(L, MT_BFSGRID);
  lua_pushcfunction(L, udGc<BFSGrid>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_BFSGrid_width);
  lua_setfield(L, -2, "width");
  lua_pushcfunction(L, l_BFSGrid_height);
  lua_setfield(L, -2, "height");
  lua_pushcfunction(L, l_BFSGrid_findPath);
  lua_setfield(L, -2, "findPath");
  lua_pushcfunction(L, l_BFSGrid_trace);
  lua_setfield(L, -2, "trace");
  lua_pushcfunction(L, l_BFSGrid_traceable);
  lua_setfield(L, -2, "traceable");
  lua_pushcfunction(L, l_BFSGrid_reset);
  lua_setfield(L, -2, "reset");
  lua_pop(L, 1);

  pushSnNamed(L, "BFSGrid");
  lua_pushcfunction(L, l_BFSGrid_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Buffer
// -----------------
static int l_Buffer_new(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  Array<void *> chunks;
  Array<size_t> chunkSizes;

  size_t n = lua_objlen(L, 1);
  chunks.reserve(n);
  chunkSizes.reserve(n);

  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));

    if (auto *v3 = udValueOrNull<Vec3>(L, -1, MT_VEC3)) {
      size_t s = sizeof(Vec3);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v3, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *v2 = udValueOrNull<Vec2>(L, -1, MT_VEC2)) {
      size_t s = sizeof(Vec2);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v2, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *cam = udValueOrNull<Camera>(L, -1, MT_CAMERA)) {
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
  udNewOwned<Buffer>(L, MT_BUFFER, std::move(buffer));
  return 1;
}
static void registerBuffer(lua_State *L) {
  luaL_newmetatable(L, MT_BUFFER);
  lua_pushcfunction(L, udGc<Buffer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  pushSnNamed(L, "Buffer");
  lua_pushcfunction(L, l_Buffer_new);
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

static int l_Rect_new(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Rect>(L, MT_RECT, Rect());
    return 1;
  }
  if (n == 4) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    udNewOwned<Rect>(L, MT_RECT, Rect(x, y, w, h));
    return 1;
  }
  if (n == 2) {
    auto &pos = udValue<Vec2>(L, 1, MT_VEC2);
    auto &size = udValue<Vec2>(L, 2, MT_VEC2);
    udNewOwned<Rect>(L, MT_RECT, Rect(pos, size));
    return 1;
  }
  if (n == 5) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float w = static_cast<float>(luaL_checknumber(L, 4));
    float h = static_cast<float>(luaL_checknumber(L, 5));
    udNewOwned<Rect>(L, MT_RECT, Rect(pivot, x, y, w, h));
    return 1;
  }
  if (n == 3) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    auto &pos = udValue<Vec2>(L, 2, MT_VEC2);
    auto &size = udValue<Vec2>(L, 3, MT_VEC2);
    udNewOwned<Rect>(L, MT_RECT, Rect(pivot, pos, size));
    return 1;
  }
  return luaL_error(L, "sn.Rect.new: invalid arguments");
}

static int l_Rect_index(lua_State *L) {
  auto &r = udValue<Rect>(L, 1, MT_RECT);
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
  luaL_getmetatable(L, MT_RECT);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int l_Rect_newindex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1, MT_RECT);
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
  return luaL_error(L, "sn.Rect: invalid field '%s'", k);
}
static void registerRect(lua_State *L) {
  luaL_newmetatable(L, MT_RECT);
  lua_pushcfunction(L, udGc<Rect>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_Rect_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Rect_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  pushSnNamed(L, "Rect");
  lua_pushcfunction(L, l_Rect_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Transform
// -----------------
static int l_Transform_new(lua_State *L) {
  udNewOwned<Transform>(L, MT_TRANSFORM, Transform());
  return 1;
}
static int l_Transform_index(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, MT_TRANSFORM);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "position") == 0) {
    udNewRef<Vec3>(L, MT_VEC3, &t.position);
    return 1;
  }
  if (std::strcmp(k, "rotation") == 0) {
    udNewRef<Vec3>(L, MT_VEC3, &t.rotation);
    return 1;
  }
  if (std::strcmp(k, "scale") == 0) {
    udNewRef<Vec3>(L, MT_VEC3, &t.scale);
    return 1;
  }
  luaL_getmetatable(L, MT_TRANSFORM);
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int l_Transform_newindex(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, MT_TRANSFORM);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3, MT_VEC3);
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
  return luaL_error(L, "sn.Transform: invalid field '%s'", k);
}
static int l_Transform_tostring(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, MT_TRANSFORM);
  String s = transformStr(t);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerTransform(lua_State *L) {
  luaL_newmetatable(L, MT_TRANSFORM);
  lua_pushcfunction(L, udGc<Transform>);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, l_Transform_index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Transform_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, l_Transform_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Transform");
  lua_pushcfunction(L, l_Transform_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Font (Ptr)
// -----------------
static int l_Font_new(lua_State *L) {
  udPushPtr<Font>(L, MT_FONT, makePtr<Font>());
  return 1;
}
static int l_Font_load(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, MT_FONT);
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
  auto &buf = udValue<Buffer>(L, 3, MT_BUFFER);
  lua_pushboolean(L, font->load(point, buf));
  return 1;
}
static int l_Font_resize(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, MT_FONT);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  font->resize(point);
  return 0;
}
static int l_Font_region(lua_State *L) {
  auto &font = udPtr<Font>(L, 1, MT_FONT);
  const char *text = luaL_checkstring(L, 2);
  int fontSize = static_cast<int>(luaL_checkinteger(L, 3));
  Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 4));
  auto &vec = udValue<Vec2>(L, 5, MT_VEC2);
  udNewOwned<Rect>(L, MT_RECT,
                   font->region(StringView(text), fontSize, pivot, vec));
  return 1;
}
static void registerFont(lua_State *L) {
  luaL_newmetatable(L, MT_FONT);
  lua_pushcfunction(L, udPtrGc<Font>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Font_load);
  lua_setfield(L, -2, "load");
  lua_pushcfunction(L, l_Font_resize);
  lua_setfield(L, -2, "resize");
  lua_pushcfunction(L, l_Font_region);
  lua_setfield(L, -2, "region");
  lua_pop(L, 1);

  pushSnNamed(L, "Font");
  lua_pushcfunction(L, l_Font_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// Texture / Model (Ptr)
// -----------------
static int l_Texture_new(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udPushPtr<Texture>(L, MT_TEXTURE, makePtr<Texture>());
    return 1;
  }
  int w = static_cast<int>(luaL_checkinteger(L, 1));
  int h = static_cast<int>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, MT_TEXTURE, makePtr<Texture>(w, h));
  return 1;
}
static int l_Texture_load(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    tex->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, MT_BUFFER);
  tex->load(buf);
  return 0;
}
static int l_Texture_loadCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  const char *path = luaL_checkstring(L, 2);
  tex->loadCubemap(StringView(path));
  return 0;
}
static int l_Texture_fill(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  auto &c = udValue<Color>(L, 2, MT_COLOR);
  tex->fill(c);
  return 0;
}
static int l_Texture_copy(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  udPushPtr<Texture>(L, MT_TEXTURE, makePtr<Texture>(tex->copy()));
  return 1;
}
static int l_Texture_size(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  udNewOwned<Vec2>(L, MT_VEC2, tex->size());
  return 1;
}
static int l_Texture_tostring(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  String s = textureStr(*tex);
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static void registerTexture(lua_State *L) {
  luaL_newmetatable(L, MT_TEXTURE);
  lua_pushcfunction(L, udPtrGc<Texture>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Texture_load);
  lua_setfield(L, -2, "load");
  lua_pushcfunction(L, l_Texture_loadCubemap);
  lua_setfield(L, -2, "loadCubemap");
  lua_pushcfunction(L, l_Texture_fill);
  lua_setfield(L, -2, "fill");
  lua_pushcfunction(L, l_Texture_copy);
  lua_setfield(L, -2, "copy");
  lua_pushcfunction(L, l_Texture_size);
  lua_setfield(L, -2, "size");
  lua_pushcfunction(L, l_Texture_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Texture");
  lua_pushcfunction(L, l_Texture_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Model_new(lua_State *L) {
  udPushPtr<Model>(L, MT_MODEL, makePtr<Model>());
  return 1;
}
static int l_Model_load(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    m->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, MT_BUFFER);
  m->load(buf);
  return 0;
}
static int l_Model_getAABB(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  udNewOwned<AABB>(L, MT_AABB, m->getAABB());
  return 1;
}
static int l_Model_loadSprite(lua_State *L) {
  udPtr<Model>(L, 1, MT_MODEL)->loadSprite();
  return 0;
}
static int l_Model_loadBox(lua_State *L) {
  udPtr<Model>(L, 1, MT_MODEL)->loadBox();
  return 0;
}
static int l_Model_getBoneUniformBuffer(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  udNewOwned<Buffer>(L, MT_BUFFER, m->getBoneUniformBuffer());
  return 1;
}
static int l_Model_play(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  float start = static_cast<float>(luaL_checknumber(L, 2));
  m->play(start);
  return 0;
}
static int l_Model_update(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  float dt = static_cast<float>(luaL_checknumber(L, 2));
  m->update(dt);
  return 0;
}
static int l_Model_hasTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  lua_pushboolean(L, m->hasTexture(k));
  return 1;
}
static int l_Model_getTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  udPushPtr<Texture>(L, MT_TEXTURE, makePtr<Texture>(m->getTexture(k)));
  return 1;
}
static int l_Model_setTexture(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  auto k = static_cast<TextureKey>(luaL_checkinteger(L, 2));
  auto &t = udPtr<Texture>(L, 3, MT_TEXTURE);
  m->setTexture(k, *t);
  return 0;
}
static void registerModel(lua_State *L) {
  luaL_newmetatable(L, MT_MODEL);
  lua_pushcfunction(L, udPtrGc<Model>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Model_load);
  lua_setfield(L, -2, "load");
  lua_pushcfunction(L, l_Model_getAABB);
  lua_setfield(L, -2, "getAABB");
  lua_pushcfunction(L, l_Model_loadSprite);
  lua_setfield(L, -2, "loadSprite");
  lua_pushcfunction(L, l_Model_loadBox);
  lua_setfield(L, -2, "loadBox");
  lua_pushcfunction(L, l_Model_getBoneUniformBuffer);
  lua_setfield(L, -2, "getBoneUniformBuffer");
  lua_pushcfunction(L, l_Model_play);
  lua_setfield(L, -2, "play");
  lua_pushcfunction(L, l_Model_update);
  lua_setfield(L, -2, "update");
  lua_pushcfunction(L, l_Model_hasTexture);
  lua_setfield(L, -2, "hasTexture");
  lua_pushcfunction(L, l_Model_getTexture);
  lua_setfield(L, -2, "getTexture");
  lua_pushcfunction(L, l_Model_setTexture);
  lua_setfield(L, -2, "setTexture");
  lua_pop(L, 1);

  pushSnNamed(L, "Model");
  lua_pushcfunction(L, l_Model_new);
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
static int l_RenderTexture_new(lua_State *L) {
  udPushPtr<RenderTexture>(L, MT_RENDERTEXTURE, makePtr<RenderTexture>());
  return 1;
}
static int l_RenderTexture_create(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, MT_RENDERTEXTURE);
  int w = static_cast<int>(luaL_checkinteger(L, 2));
  int h = static_cast<int>(luaL_checkinteger(L, 3));
  rt->create(w, h);
  return 0;
}
static void registerRenderTexture(lua_State *L) {
  luaL_newmetatable(L, MT_RENDERTEXTURE);
  lua_pushcfunction(L, udPtrGc<RenderTexture>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_RenderTexture_create);
  lua_setfield(L, -2, "create");
  lua_pop(L, 1);

  pushSnNamed(L, "RenderTexture");
  lua_pushcfunction(L, l_RenderTexture_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Sound_new(lua_State *L) {
  udPushPtr<Sound>(L, MT_SOUND, makePtr<Sound>());
  return 1;
}
static int l_Sound_load(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    s->load(StringView(path));
    return 0;
  }
  auto &buf = udValue<Buffer>(L, 2, MT_BUFFER);
  s->load(buf);
  return 0;
}
static int l_Sound_play(lua_State *L) {
  udPtr<Sound>(L, 1, MT_SOUND)->play();
  return 0;
}
static int l_Sound_restart(lua_State *L) {
  udPtr<Sound>(L, 1, MT_SOUND)->restart();
  return 0;
}
static int l_Sound_stop(lua_State *L) {
  udPtr<Sound>(L, 1, MT_SOUND)->stop();
  return 0;
}
static int l_Sound_setLooping(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  bool looping = lua_toboolean(L, 2) != 0;
  s->setLooping(looping);
  return 0;
}
static int l_Sound_setVolume(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setVolume(v);
  return 0;
}
static int l_Sound_setPitch(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  s->setPitch(v);
  return 0;
}
static int l_Sound_setPosition(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  auto &p = udValue<Vec3>(L, 2, MT_VEC3);
  s->setPosition(p);
  return 0;
}
static int l_Sound_setDirection(lua_State *L) {
  auto &s = udPtr<Sound>(L, 1, MT_SOUND);
  auto &d = udValue<Vec3>(L, 2, MT_VEC3);
  s->setDirection(d);
  return 0;
}
static int l_Sound_isPlaying(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1, MT_SOUND)->isPlaying());
  return 1;
}
static int l_Sound_isLooping(lua_State *L) {
  lua_pushboolean(L, udPtr<Sound>(L, 1, MT_SOUND)->isLooping());
  return 1;
}
static int l_Sound_getVolume(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1, MT_SOUND)->getVolume());
  return 1;
}
static int l_Sound_getPitch(lua_State *L) {
  lua_pushnumber(L, udPtr<Sound>(L, 1, MT_SOUND)->getPitch());
  return 1;
}
static int l_Sound_getPosition(lua_State *L) {
  udNewOwned<Vec3>(L, MT_VEC3, udPtr<Sound>(L, 1, MT_SOUND)->getPosition());
  return 1;
}
static int l_Sound_getDirection(lua_State *L) {
  udNewOwned<Vec3>(L, MT_VEC3, udPtr<Sound>(L, 1, MT_SOUND)->getDirection());
  return 1;
}
static void registerSound(lua_State *L) {
  luaL_newmetatable(L, MT_SOUND);
  lua_pushcfunction(L, udPtrGc<Sound>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Sound_load);
  lua_setfield(L, -2, "load");
  lua_pushcfunction(L, l_Sound_play);
  lua_setfield(L, -2, "play");
  lua_pushcfunction(L, l_Sound_restart);
  lua_setfield(L, -2, "restart");
  lua_pushcfunction(L, l_Sound_stop);
  lua_setfield(L, -2, "stop");
  lua_pushcfunction(L, l_Sound_setLooping);
  lua_setfield(L, -2, "setLooping");
  lua_pushcfunction(L, l_Sound_setVolume);
  lua_setfield(L, -2, "setVolume");
  lua_pushcfunction(L, l_Sound_setPitch);
  lua_setfield(L, -2, "setPitch");
  lua_pushcfunction(L, l_Sound_setPosition);
  lua_setfield(L, -2, "setPosition");
  lua_pushcfunction(L, l_Sound_setDirection);
  lua_setfield(L, -2, "setDirection");
  lua_pushcfunction(L, l_Sound_isPlaying);
  lua_setfield(L, -2, "isPlaying");
  lua_pushcfunction(L, l_Sound_isLooping);
  lua_setfield(L, -2, "isLooping");
  lua_pushcfunction(L, l_Sound_getVolume);
  lua_setfield(L, -2, "getVolume");
  lua_pushcfunction(L, l_Sound_getPitch);
  lua_setfield(L, -2, "getPitch");
  lua_pushcfunction(L, l_Sound_getPosition);
  lua_setfield(L, -2, "getPosition");
  lua_pushcfunction(L, l_Sound_getDirection);
  lua_setfield(L, -2, "getDirection");
  lua_pop(L, 1);

  pushSnNamed(L, "Sound");
  lua_pushcfunction(L, l_Sound_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Shader_new(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER, makePtr<Shader>());
  return 1;
}
static int l_Shader_load(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1, MT_SHADER);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  int numUniformData = static_cast<int>(luaL_checkinteger(L, 4));
  s->load(StringView(name), stage, numUniformData);
  return 0;
}
static int l_Shader_compileAndLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1, MT_SHADER);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  s->compileAndLoad(StringView(name), stage);
  return 0;
}
static void registerShader(lua_State *L) {
  luaL_newmetatable(L, MT_SHADER);
  lua_pushcfunction(L, udPtrGc<Shader>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Shader_load);
  lua_setfield(L, -2, "load");
  lua_pushcfunction(L, l_Shader_compileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  lua_pushcfunction(L, l_Shader_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

static int l_Pipeline_new(lua_State *L) {
  udPushPtr<GraphicsPipeline>(L, MT_PIPELINE, makePtr<GraphicsPipeline>());
  return 1;
}
static int l_Pipeline_setVertexShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  auto &s = udPtr<Shader>(L, 2, MT_SHADER);
  p->setVertexShader(*s);
  return 0;
}
static int l_Pipeline_setFragmentShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  auto &s = udPtr<Shader>(L, 2, MT_SHADER);
  p->setFragmentShader(*s);
  return 0;
}
static int l_Pipeline_setEnableDepthTest(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableDepthTest(enable);
  return 0;
}
static int l_Pipeline_setEnableInstanced(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableInstanced(enable);
  return 0;
}
static int l_Pipeline_setEnableAnimation(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableAnimation(enable);
  return 0;
}
static int l_Pipeline_setEnableTangent(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableTangent(enable);
  return 0;
}
static int l_Pipeline_build(lua_State *L) {
  udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE)->build();
  return 0;
}
static void registerPipeline(lua_State *L) {
  luaL_newmetatable(L, MT_PIPELINE);
  lua_pushcfunction(L, udPtrGc<GraphicsPipeline>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, l_Pipeline_setVertexShader);
  lua_setfield(L, -2, "setVertexShader");
  lua_pushcfunction(L, l_Pipeline_setFragmentShader);
  lua_setfield(L, -2, "setFragmentShader");
  lua_pushcfunction(L, l_Pipeline_setEnableDepthTest);
  lua_setfield(L, -2, "setEnableDepthTest");
  lua_pushcfunction(L, l_Pipeline_setEnableInstanced);
  lua_setfield(L, -2, "setEnableInstanced");
  lua_pushcfunction(L, l_Pipeline_setEnableAnimation);
  lua_setfield(L, -2, "setEnableAnimation");
  lua_pushcfunction(L, l_Pipeline_setEnableTangent);
  lua_setfield(L, -2, "setEnableTangent");
  lua_pushcfunction(L, l_Pipeline_build);
  lua_setfield(L, -2, "build");
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  lua_pushcfunction(L, l_Pipeline_new);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

// -----------------
// sn.* static modules
// -----------------
static int l_Arguments_getArgc(lua_State *L) {
  lua_pushinteger(L, Arguments::getArgc());
  return 1;
}
static int l_Arguments_getArgv(lua_State *L) {
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
  lua_pushcfunction(L, l_Arguments_getArgc);
  lua_setfield(L, -2, "getArgc");
  lua_pushcfunction(L, l_Arguments_getArgv);
  lua_setfield(L, -2, "getArgv");
  lua_pop(L, 1);
}

static int l_Random_getRange(lua_State *L) {
  float min = static_cast<float>(luaL_checknumber(L, 1));
  float max = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Random::getRange(min, max));
  return 1;
}
static int l_Random_getIntRange(lua_State *L) {
  int min = static_cast<int>(luaL_checkinteger(L, 1));
  int max = static_cast<int>(luaL_checkinteger(L, 2));
  lua_pushinteger(L, Random::getIntRange(min, max));
  return 1;
}
static void registerRandom(lua_State *L) {
  pushSnNamed(L, "Random");
  lua_pushcfunction(L, l_Random_getRange);
  lua_setfield(L, -2, "getRange");
  lua_pushcfunction(L, l_Random_getIntRange);
  lua_setfield(L, -2, "getIntRange");
  lua_pop(L, 1);
}

static int l_Window_getName(lua_State *L) {
  auto name = Window::name();
  lua_pushlstring(L, name.data(), name.size());
  return 1;
}
static int l_Window_size(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::size());
  return 1;
}
static int l_Window_half(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::half());
  return 1;
}
static int l_Window_resize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1, MT_VEC2);
  Window::resize(size);
  return 0;
}
static int l_Window_setFullscreen(lua_State *L) {
  bool fs = lua_toboolean(L, 1) != 0;
  Window::setFullscreen(fs);
  return 0;
}
static int l_Window_rename(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  Window::rename(StringView(name));
  return 0;
}
static int l_Window_resized(lua_State *L) {
  lua_pushboolean(L, Window::resized());
  return 1;
}
static int l_Window_rect(lua_State *L) {
  udNewOwned<Rect>(L, MT_RECT, Window::rect());
  return 1;
}
static int l_Window_topLeft(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::topLeft());
  return 1;
}
static int l_Window_topCenter(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::topCenter());
  return 1;
}
static int l_Window_topRight(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::topRight());
  return 1;
}
static int l_Window_bottomLeft(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::bottomLeft());
  return 1;
}
static int l_Window_bottomCenter(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::bottomCenter());
  return 1;
}
static int l_Window_bottomRight(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Window::bottomRight());
  return 1;
}
static void registerWindow(lua_State *L) {
  pushSnNamed(L, "Window");
  lua_pushcfunction(L, l_Window_getName);
  lua_setfield(L, -2, "getName");
  lua_pushcfunction(L, l_Window_size);
  lua_setfield(L, -2, "size");
  lua_pushcfunction(L, l_Window_half);
  lua_setfield(L, -2, "half");
  lua_pushcfunction(L, l_Window_resize);
  lua_setfield(L, -2, "resize");
  lua_pushcfunction(L, l_Window_setFullscreen);
  lua_setfield(L, -2, "setFullscreen");
  lua_pushcfunction(L, l_Window_rename);
  lua_setfield(L, -2, "rename");
  lua_pushcfunction(L, l_Window_resized);
  lua_setfield(L, -2, "resized");
  lua_pushcfunction(L, l_Window_rect);
  lua_setfield(L, -2, "rect");
  lua_pushcfunction(L, l_Window_topLeft);
  lua_setfield(L, -2, "topLeft");
  lua_pushcfunction(L, l_Window_topCenter);
  lua_setfield(L, -2, "topCenter");
  lua_pushcfunction(L, l_Window_topRight);
  lua_setfield(L, -2, "topRight");
  lua_pushcfunction(L, l_Window_bottomLeft);
  lua_setfield(L, -2, "bottomLeft");
  lua_pushcfunction(L, l_Window_bottomCenter);
  lua_setfield(L, -2, "bottomCenter");
  lua_pushcfunction(L, l_Window_bottomRight);
  lua_setfield(L, -2, "bottomRight");
  lua_pop(L, 1);
}

static int l_Physics_createBoxCollider(lua_State *L) {
  auto &t = udValue<Transform>(L, 1, MT_TRANSFORM);
  bool isStatic = lua_toboolean(L, 2) != 0;
  udNewOwned<Collider>(L, MT_COLLIDER, Physics::createBoxCollider(t, isStatic));
  return 1;
}
static int l_Physics_createSphereCollider(lua_State *L) {
  auto &pos = udValue<Vec3>(L, 1, MT_VEC3);
  float radius = static_cast<float>(luaL_checknumber(L, 2));
  bool isStatic = lua_toboolean(L, 3) != 0;
  udNewOwned<Collider>(L, MT_COLLIDER,
                       Physics::createSphereCollider(pos, radius, isStatic));
  return 1;
}
static int l_Physics_createCylinderCollider(lua_State *L) {
  auto &pos = udValue<Vec3>(L, 1, MT_VEC3);
  auto &rot = udValue<Vec3>(L, 2, MT_VEC3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 3));
  float radius = static_cast<float>(luaL_checknumber(L, 4));
  bool isStatic = lua_toboolean(L, 5) != 0;
  udNewOwned<Collider>(
      L, MT_COLLIDER,
      Physics::createCylinderCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}
static int l_Physics_addCollider(lua_State *L) {
  auto &c = udValue<Collider>(L, 1, MT_COLLIDER);
  bool active = lua_toboolean(L, 2) != 0;
  Physics::addCollider(c, active);
  return 0;
}
static void registerPhysics(lua_State *L) {
  pushSnNamed(L, "Physics");
  lua_pushcfunction(L, l_Physics_createBoxCollider);
  lua_setfield(L, -2, "createBoxCollider");
  lua_pushcfunction(L, l_Physics_createSphereCollider);
  lua_setfield(L, -2, "createSphereCollider");
  lua_pushcfunction(L, l_Physics_createCylinderCollider);
  lua_setfield(L, -2, "createCylinderCollider");
  lua_pushcfunction(L, l_Physics_addCollider);
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

static int l_BuiltinShader_getDefaultVS(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER,
                    makePtr<Shader>(BuiltinShader::getDefaultVS()));
  return 1;
}
static int l_BuiltinShader_getDefaultFS(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER,
                    makePtr<Shader>(BuiltinShader::getDefaultFS()));
  return 1;
}
static int l_BuiltinShader_getDefaultInstancedVS(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER,
                    makePtr<Shader>(BuiltinShader::getDefaultInstancedVS()));
  return 1;
}
static int l_BuiltinShader_getCubemapVS(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER,
                    makePtr<Shader>(BuiltinShader::getCubemapVS()));
  return 1;
}
static int l_BuiltinShader_getCubemapFS(lua_State *L) {
  udPushPtr<Shader>(L, MT_SHADER,
                    makePtr<Shader>(BuiltinShader::getCubemapFS()));
  return 1;
}
static void registerBuiltinShader(lua_State *L) {
  pushSnNamed(L, "BuiltinShader");
  lua_pushcfunction(L, l_BuiltinShader_getDefaultVS);
  lua_setfield(L, -2, "getDefaultVS");
  lua_pushcfunction(L, l_BuiltinShader_getDefaultFS);
  lua_setfield(L, -2, "getDefaultFS");
  lua_pushcfunction(L, l_BuiltinShader_getDefaultInstancedVS);
  lua_setfield(L, -2, "getDefaultInstancedVS");
  lua_pushcfunction(L, l_BuiltinShader_getCubemapVS);
  lua_setfield(L, -2, "getCubemapVS");
  lua_pushcfunction(L, l_BuiltinShader_getCubemapFS);
  lua_setfield(L, -2, "getCubemapFS");
  lua_pop(L, 1);
}

static int l_BuiltinPipeline_getDefault3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, MT_PIPELINE,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault3D()));
  return 1;
}
static int l_BuiltinPipeline_getInstanced3D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, MT_PIPELINE,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getInstanced3D()));
  return 1;
}
static int l_BuiltinPipeline_getDefault2D(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, MT_PIPELINE,
      makePtr<GraphicsPipeline>(BuiltinPipeline::getDefault2D()));
  return 1;
}
static int l_BuiltinPipeline_getCubemap(lua_State *L) {
  udPushPtr<GraphicsPipeline>(
      L, MT_PIPELINE, makePtr<GraphicsPipeline>(BuiltinPipeline::getCubemap()));
  return 1;
}
static void registerBuiltinPipeline(lua_State *L) {
  pushSnNamed(L, "BuiltinPipeline");
  lua_pushcfunction(L, l_BuiltinPipeline_getDefault3D);
  lua_setfield(L, -2, "getDefault3D");
  lua_pushcfunction(L, l_BuiltinPipeline_getInstanced3D);
  lua_setfield(L, -2, "getInstanced3D");
  lua_pushcfunction(L, l_BuiltinPipeline_getDefault2D);
  lua_setfield(L, -2, "getDefault2D");
  lua_pushcfunction(L, l_BuiltinPipeline_getCubemap);
  lua_setfield(L, -2, "getCubemap");
  lua_pop(L, 1);
}

static int l_Event_quit(lua_State *L) {
  (void)L;
  Event::quit();
  return 0;
}
static void registerEvent(lua_State *L) {
  pushSnNamed(L, "Event");
  lua_pushcfunction(L, l_Event_quit);
  lua_setfield(L, -2, "quit");
  lua_pop(L, 1);
}

static int l_Keyboard_isPressed(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isPressed(key));
  return 1;
}
static int l_Keyboard_isReleased(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isReleased(key));
  return 1;
}
static int l_Keyboard_isDown(lua_State *L) {
  auto key = static_cast<Keyboard::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Keyboard::isDown(key));
  return 1;
}
static void registerKeyboard(lua_State *L) {
  pushSnNamed(L, "Keyboard");
  lua_pushcfunction(L, l_Keyboard_isPressed);
  lua_setfield(L, -2, "isPressed");
  lua_pushcfunction(L, l_Keyboard_isReleased);
  lua_setfield(L, -2, "isReleased");
  lua_pushcfunction(L, l_Keyboard_isDown);
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

static int l_Gamepad_isPressed(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isPressed(btn));
  return 1;
}
static int l_Gamepad_isReleased(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isReleased(btn));
  return 1;
}
static int l_Gamepad_isDown(lua_State *L) {
  auto btn = static_cast<GamePad::code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, GamePad::isDown(btn));
  return 1;
}
static int l_Gamepad_getLeftStick(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, GamePad::getLeftStick());
  return 1;
}
static int l_Gamepad_getRightStick(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, GamePad::getRightStick());
  return 1;
}
static int l_Gamepad_isConnected(lua_State *L) {
  lua_pushboolean(L, GamePad::isConnected());
  return 1;
}
static void registerGamepad(lua_State *L) {
  pushSnNamed(L, "Gamepad");
  lua_pushcfunction(L, l_Gamepad_isPressed);
  lua_setfield(L, -2, "isPressed");
  lua_pushcfunction(L, l_Gamepad_isReleased);
  lua_setfield(L, -2, "isReleased");
  lua_pushcfunction(L, l_Gamepad_isDown);
  lua_setfield(L, -2, "isDown");
  lua_pushcfunction(L, l_Gamepad_getLeftStick);
  lua_setfield(L, -2, "getLeftStick");
  lua_pushcfunction(L, l_Gamepad_getRightStick);
  lua_setfield(L, -2, "getRightStick");
  lua_pushcfunction(L, l_Gamepad_isConnected);
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

static int l_Graphics_drawRect(lua_State *L) {
  auto &rect = udValue<Rect>(L, 1, MT_RECT);
  auto &color = udValue<Color>(L, 2, MT_COLOR);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawRect(rect, color, angle);
    return 0;
  }
  Graphics::drawRect(rect, color);
  return 0;
}
static int l_Graphics_drawText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &font = udPtr<Font>(L, 2, MT_FONT);
  auto &pos = udValue<Vec2>(L, 3, MT_VEC2);
  if (lua_gettop(L) == 3) {
    Graphics::drawText(StringView(text), *font, pos);
    return 0;
  }
  auto &color = udValue<Color>(L, 4, MT_COLOR);
  float size = static_cast<float>(luaL_optnumber(L, 5, 32.0));
  float angle = static_cast<float>(luaL_optnumber(L, 6, 0.0));
  Graphics::drawText(StringView(text), *font, pos, color, size, angle);
  return 0;
}
static int l_Graphics_drawImage(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  auto &rect = udValue<Rect>(L, 2, MT_RECT);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawImage(*tex, rect, angle);
    return 0;
  }
  Graphics::drawImage(*tex, rect);
  return 0;
}
static int l_Graphics_drawCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1, MT_TEXTURE);
  Graphics::drawCubemap(*tex);
  return 0;
}
static int l_Graphics_drawModel(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  auto &t = udValue<Transform>(L, 2, MT_TRANSFORM);
  Graphics::drawModel(*m, t);
  return 0;
}
static int l_Graphics_drawModelInstanced(lua_State *L) {
  auto &m = udPtr<Model>(L, 1, MT_MODEL);
  luaL_checktype(L, 2, LUA_TTABLE);
  Array<Transform> transforms;
  size_t n = lua_objlen(L, 2);
  transforms.reserve(n);
  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 2, static_cast<lua_Integer>(i));
    transforms.push_back(udValue<Transform>(L, -1, MT_TRANSFORM));
    lua_pop(L, 1);
  }
  Graphics::drawModelInstanced(*m, transforms);
  return 0;
}
static int l_Graphics_setCamera(lua_State *L) {
  auto &cam = udValue<Camera>(L, 1, MT_CAMERA);
  Graphics::setCamera(cam);
  return 0;
}
static int l_Graphics_getCamera(lua_State *L) {
  auto &cam = Graphics::getCamera();
  udNewRef<Camera>(L, MT_CAMERA, &cam);
  return 1;
}
static int l_Graphics_setCamera2d(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1, MT_CAMERA2D);
  Graphics::setCamera2D(cam);
  return 0;
}
static int l_Graphics_getCamera2d(lua_State *L) {
  auto &cam = Graphics::getCamera2D();
  udNewRef<Camera2D>(L, MT_CAMERA2D, &cam);
  return 1;
}
static int l_Graphics_getClearColor(lua_State *L) {
  udNewOwned<Color>(L, MT_COLOR, Graphics::getClearColor());
  return 1;
}
static int l_Graphics_setClearColor(lua_State *L) {
  auto &c = udValue<Color>(L, 1, MT_COLOR);
  Graphics::setClearColor(c);
  return 0;
}
static int l_Graphics_setGraphicsPipeline(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1, MT_PIPELINE);
  Graphics::setGraphicsPipeline(*p);
  return 0;
}
static int l_Graphics_resetGraphicsPipeline(lua_State *L) {
  (void)L;
  Graphics::resetGraphicsPipeline();
  return 0;
}
static int l_Graphics_setTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &t = udPtr<Texture>(L, 2, MT_TEXTURE);
  Graphics::setTexture(slot, *t);
  return 0;
}
static int l_Graphics_resetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Graphics::resetTexture(slot);
  return 0;
}
static int l_Graphics_resetAllTexture(lua_State *L) {
  (void)L;
  Graphics::resetAllTexture();
  return 0;
}
static int l_Graphics_setUniformBuffer(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &b = udValue<Buffer>(L, 2, MT_BUFFER);
  Graphics::setUniformBuffer(slot, b);
  return 0;
}
static int l_Graphics_setRenderTarget(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, MT_RENDERTEXTURE);
  Graphics::setRenderTarget(*rt);
  return 0;
}
static int l_Graphics_flush(lua_State *L) {
  (void)L;
  Graphics::flush();
  return 0;
}
static int l_Graphics_readbackTexture(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1, MT_RENDERTEXTURE);
  auto &out = udPtr<Texture>(L, 2, MT_TEXTURE);
  lua_pushboolean(L, Graphics::readbackTexture(*rt, *out));
  return 1;
}
static void registerGraphics(lua_State *L) {
  pushSnNamed(L, "Graphics");
  lua_pushcfunction(L, l_Graphics_drawRect);
  lua_setfield(L, -2, "drawRect");
  lua_pushcfunction(L, l_Graphics_drawText);
  lua_setfield(L, -2, "drawText");
  lua_pushcfunction(L, l_Graphics_drawImage);
  lua_setfield(L, -2, "drawImage");
  lua_pushcfunction(L, l_Graphics_drawCubemap);
  lua_setfield(L, -2, "drawCubemap");
  lua_pushcfunction(L, l_Graphics_drawModel);
  lua_setfield(L, -2, "drawModel");
  lua_pushcfunction(L, l_Graphics_drawModelInstanced);
  lua_setfield(L, -2, "drawModelInstanced");
  lua_pushcfunction(L, l_Graphics_setCamera);
  lua_setfield(L, -2, "setCamera");
  lua_pushcfunction(L, l_Graphics_getCamera);
  lua_setfield(L, -2, "getCamera");
  lua_pushcfunction(L, l_Graphics_setCamera2d);
  lua_setfield(L, -2, "setCamera2d");
  lua_pushcfunction(L, l_Graphics_getCamera2d);
  lua_setfield(L, -2, "getCamera2d");
  lua_pushcfunction(L, l_Graphics_getClearColor);
  lua_setfield(L, -2, "getClearColor");
  lua_pushcfunction(L, l_Graphics_setClearColor);
  lua_setfield(L, -2, "setClearColor");
  lua_pushcfunction(L, l_Graphics_setGraphicsPipeline);
  lua_setfield(L, -2, "setGraphicsPipeline");
  lua_pushcfunction(L, l_Graphics_resetGraphicsPipeline);
  lua_setfield(L, -2, "resetGraphicsPipeline");
  lua_pushcfunction(L, l_Graphics_setTexture);
  lua_setfield(L, -2, "setTexture");
  lua_pushcfunction(L, l_Graphics_resetTexture);
  lua_setfield(L, -2, "resetTexture");
  lua_pushcfunction(L, l_Graphics_resetAllTexture);
  lua_setfield(L, -2, "resetAllTexture");
  lua_pushcfunction(L, l_Graphics_setUniformBuffer);
  lua_setfield(L, -2, "setUniformBuffer");
  lua_pushcfunction(L, l_Graphics_setRenderTarget);
  lua_setfield(L, -2, "setRenderTarget");
  lua_pushcfunction(L, l_Graphics_flush);
  lua_setfield(L, -2, "flush");
  lua_pushcfunction(L, l_Graphics_readbackTexture);
  lua_setfield(L, -2, "readbackTexture");
  lua_pop(L, 1);
}

static int l_Mouse_getPositionOnScene(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Mouse::getPositionOnScene());
  return 1;
}
static int l_Mouse_getPosition(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Mouse::getPosition());
  return 1;
}
static int l_Mouse_isPressed(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isPressed(b));
  return 1;
}
static int l_Mouse_isReleased(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isReleased(b));
  return 1;
}
static int l_Mouse_isDown(lua_State *L) {
  auto b = static_cast<Mouse::Code>(luaL_checkinteger(L, 1));
  lua_pushboolean(L, Mouse::isDown(b));
  return 1;
}
static int l_Mouse_setPosition(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1, MT_VEC2);
  Mouse::setPosition(p);
  return 0;
}
static int l_Mouse_setPositionOnScene(lua_State *L) {
  auto &p = udValue<Vec2>(L, 1, MT_VEC2);
  Mouse::setPositionOnScene(p);
  return 0;
}
static int l_Mouse_getScrollWheel(lua_State *L) {
  udNewOwned<Vec2>(L, MT_VEC2, Mouse::getScrollWheel());
  return 1;
}
static int l_Mouse_hideCursor(lua_State *L) {
  bool hide = lua_toboolean(L, 1) != 0;
  Mouse::hideCursor(hide);
  return 0;
}
static int l_Mouse_setRelative(lua_State *L) {
  bool rel = lua_toboolean(L, 1) != 0;
  Mouse::setRelative(rel);
  return 0;
}
static int l_Mouse_isRelative(lua_State *L) {
  lua_pushboolean(L, Mouse::isRelative());
  return 1;
}
static void registerMouse(lua_State *L) {
  pushSnNamed(L, "Mouse");
  lua_pushcfunction(L, l_Mouse_setRelative);
  lua_setfield(L, -2, "setRelative");
  lua_pushcfunction(L, l_Mouse_isRelative);
  lua_setfield(L, -2, "isRelative");
  lua_pushcfunction(L, l_Mouse_getPositionOnScene);
  lua_setfield(L, -2, "getPositionOnScene");
  lua_pushcfunction(L, l_Mouse_getPosition);
  lua_setfield(L, -2, "getPosition");
  lua_pushcfunction(L, l_Mouse_isPressed);
  lua_setfield(L, -2, "isPressed");
  lua_pushcfunction(L, l_Mouse_isReleased);
  lua_setfield(L, -2, "isReleased");
  lua_pushcfunction(L, l_Mouse_isDown);
  lua_setfield(L, -2, "isDown");
  lua_pushcfunction(L, l_Mouse_setPosition);
  lua_setfield(L, -2, "setPosition");
  lua_pushcfunction(L, l_Mouse_setPositionOnScene);
  lua_setfield(L, -2, "setPositionOnScene");
  lua_pushcfunction(L, l_Mouse_getScrollWheel);
  lua_setfield(L, -2, "getScrollWheel");
  lua_pushcfunction(L, l_Mouse_hideCursor);
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

static int l_Filesystem_enumerateDirectory(lua_State *L) {
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
  lua_pushcfunction(L, l_Filesystem_enumerateDirectory);
  lua_setfield(L, -2, "enumerateDirectory");
  lua_pop(L, 1);
}

static int l_Script_load(lua_State *L) {
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
  lua_pushcfunction(L, l_Script_load);
  lua_setfield(L, -2, "load");
  lua_pop(L, 1);
}

static int l_Logger_info(lua_State *L) {
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, 1);
  if (luaPCallLogged(L, 1, 1) != LUA_OK) {
    return 0;
  }
  const char *s = lua_tostring(L, -1);
  Logger::info("%s", s ? s : "");
  lua_pop(L, 1);
  return 0;
}
static int l_Logger_verbose(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Logger::verbose("%s", s);
  return 0;
}
static int l_Logger_error(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Logger::error("%s", s);
  return 0;
}
static int l_Logger_warn(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Logger::warn("%s", s);
  return 0;
}
static int l_Logger_critical(lua_State *L) {
  const char *s = luaL_checkstring(L, 1);
  Logger::critical("%s", s);
  return 0;
}
static void registerLogger(lua_State *L) {
  pushSnNamed(L, "Logger");
  lua_pushcfunction(L, l_Logger_verbose);
  lua_setfield(L, -2, "verbose");
  lua_pushcfunction(L, l_Logger_info);
  lua_setfield(L, -2, "info");
  lua_pushcfunction(L, l_Logger_error);
  lua_setfield(L, -2, "error");
  lua_pushcfunction(L, l_Logger_warn);
  lua_setfield(L, -2, "warn");
  lua_pushcfunction(L, l_Logger_critical);
  lua_setfield(L, -2, "critical");
  lua_pop(L, 1);
}

static int l_Periodic_sin0_1(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::sineWave(period, t));
  return 1;
}
static int l_Periodic_cos0_1(lua_State *L) {
  float period = static_cast<float>(luaL_checknumber(L, 1));
  float t = static_cast<float>(luaL_checknumber(L, 2));
  lua_pushnumber(L, Periodic::cosWave(period, t));
  return 1;
}
static void registerPeriodic(lua_State *L) {
  pushSnNamed(L, "Periodic");
  lua_pushcfunction(L, l_Periodic_sin0_1);
  lua_setfield(L, -2, "sin0_1");
  lua_pushcfunction(L, l_Periodic_cos0_1);
  lua_setfield(L, -2, "cos0_1");
  lua_pop(L, 1);
}

static int l_Time_seconds(lua_State *L) {
  lua_pushnumber(L, Time::seconds());
  return 1;
}
static int l_Time_milli(lua_State *L) {
  lua_pushinteger(L, static_cast<lua_Integer>(Time::milli()));
  return 1;
}
static int l_Time_delta(lua_State *L) {
  lua_pushnumber(L, Time::deltaTime());
  return 1;
}
static void registerTime(lua_State *L) {
  pushSnNamed(L, "Time");
  lua_pushcfunction(L, l_Time_seconds);
  lua_setfield(L, -2, "seconds");
  lua_pushcfunction(L, l_Time_milli);
  lua_setfield(L, -2, "milli");
  lua_pushcfunction(L, l_Time_delta);
  lua_setfield(L, -2, "delta");
  lua_pop(L, 1);
}

static int l_ImGui_Begin(lua_State *L) {
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
static int l_ImGui_End(lua_State *L) {
  (void)L;
  ImGui::End();
  return 0;
}
static int l_ImGui_button(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_pushboolean(L, ImGui::Button(name));
  return 1;
}
static int l_ImGui_text(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  ImGui::Text("%s", text);
  return 0;
}
static int l_ImGui_setNextWindowPos(lua_State *L) {
  auto &pos = udValue<Vec2>(L, 1, MT_VEC2);
  ImGui::SetNextWindowPos({pos.x, pos.y});
  return 0;
}
static int l_ImGui_setNextWindowSize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1, MT_VEC2);
  ImGui::SetNextWindowSize({size.x, size.y});
  return 0;
}
static void registerImGui(lua_State *L) {
  pushSnNamed(L, "ImGui");
  lua_pushcfunction(L, l_ImGui_Begin);
  lua_setfield(L, -2, "Begin");
  lua_pushcfunction(L, l_ImGui_End);
  lua_setfield(L, -2, "End");
  lua_pushcfunction(L, l_ImGui_button);
  lua_setfield(L, -2, "button");
  lua_pushcfunction(L, l_ImGui_text);
  lua_setfield(L, -2, "text");
  lua_pushcfunction(L, l_ImGui_setNextWindowPos);
  lua_setfield(L, -2, "setNextWindowPos");
  lua_pushcfunction(L, l_ImGui_setNextWindowSize);
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

static int l_import(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  String filename = String(name) + ".lua";
  String source = AssetIO::openAsString(filename);
  if (source.empty()) {
    lua_pushnil(L);
    return 1;
  }
  String chunkname = "@" + AssetIO::getFilePath(filename);
  if (luaL_loadbuffer(L, source.data(), source.size(), chunkname.c_str()) !=
      LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    Logger::error("[lua load error] %s", msg ? msg : "(unknown error)");
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
  lua_pushcfunction(L, l_import);
  lua_setglobal(L, "import");

  registerVec2(L);
  registerVec3(L);
  registerColor(L);
  registerAABB(L);
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
  registerLogger(L);
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
    Logger::error("lua_newstate failed");
    return false;
  }
  luaL_openlibs(gLua);
  lua_atpanic(gLua, [](lua_State *L) -> int {
    const char *msg = lua_tostring(L, -1);
    Logger::critical("[lua panic] %s", msg ? msg : "(unknown error)");
    return 0;
  });

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
    luaL_unref(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaL_unref(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaL_unref(gLua, LUA_REGISTRYINDEX, gDrawRef);
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
    Logger::error("[lua error] %s", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
  };

  String source;
  source = AssetIO::openAsString(String(sceneName) + ".lua");
  if (source.empty()) {
    source = nothingSceneLua;
  }

  if (gSetupRef != LUA_NOREF) {
    luaL_unref(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaL_unref(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaL_unref(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }

  String filename = String(sceneName) + ".lua";
  String chunkname = "@" + AssetIO::getFilePath(filename);
  if (luaL_loadbuffer(gLua, source.data(), source.size(), chunkname.c_str()) !=
      LUA_OK) {
    logPCallError(gLua);
    return;
  }
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    logPCallError(gLua);
    return;
  }

  lua_getglobal(gLua, "setup");
  if (lua_isfunction(gLua, -1)) {
    gSetupRef = luaL_ref(gLua, LUA_REGISTRYINDEX);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "update");
  if (lua_isfunction(gLua, -1)) {
    gUpdateRef = luaL_ref(gLua, LUA_REGISTRYINDEX);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "draw");
  if (lua_isfunction(gLua, -1)) {
    gDrawRef = luaL_ref(gLua, LUA_REGISTRYINDEX);
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
    Logger::error("[lua error] %s", msg ? msg : "(unknown error)");
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
    Logger::error("[lua error] %s", msg ? msg : "(unknown error)");
    lua_pop(gLua, 1);
  }
#endif
}

} // namespace sinen
