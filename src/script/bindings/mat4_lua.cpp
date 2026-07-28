#include "luaapi.hpp"
#include <math/matrix.hpp>

namespace sinen {
static int lMat4New(lua_State *L) {
  const int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Mat4>(L, Mat4::identity());
    return 1;
  }
  if (n == 1) {
    const float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Mat4>(L, Mat4(v));
    return 1;
  }
  if (n == 16) {
    float values[16];
    for (int i = 0; i < 16; ++i) {
      values[i] = static_cast<float>(luaL_checknumber(L, i + 1));
    }
    udNewOwned<Mat4>(L, Mat4(values));
    return 1;
  }
  return luaLError2(L, "sn.Mat4.new expects 0, 1, or 16 numbers");
}

static int lMat4Mul(lua_State *L) {
  const auto &a = udValue<Mat4>(L, 1);
  const auto &b = udValue<Mat4>(L, 2);
  udNewOwned<Mat4>(L, a * b);
  return 1;
}

static int lMat4Copy(lua_State *L) {
  const auto &m = udValue<Mat4>(L, 1);
  udNewOwned<Mat4>(L, m);
  return 1;
}

static int lMat4Transpose(lua_State *L) {
  const auto &m = udValue<Mat4>(L, 1);
  udNewOwned<Mat4>(L, Mat4::transpose(m));
  return 1;
}

static int lMat4Identity(lua_State *L) {
  udNewOwned<Mat4>(L, Mat4::identity());
  return 1;
}

void registerMat4(lua_State *L) {
  luaL_newmetatable(L, Mat4::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Mat4>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "__mul", lMat4Mul);
  Binding::registerFunction(L, "copy", lMat4Copy);
  Binding::registerFunction(L, "transpose", lMat4Transpose);
  lua_pop(L, 1);

  pushSnNamed(L, "Mat4");
  Binding::registerFunction(L, "new", lMat4New);
  Binding::registerFunction(L, "identity", lMat4Identity);
  lua_pop(L, 1);
}
} // namespace sinen
