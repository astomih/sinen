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

static auto lMat4Mul(const Mat4 &a, const Mat4 &b) { return a * b; }

static auto lMat4Copy(const Mat4 &m) { return m; }

static auto lMat4Transpose(const Mat4 &m) { return Mat4::transpose(m); }

static auto lMat4Identity() { return Mat4::identity(); }

void registerMat4(luau::table &sn) {
  auto mat4 = engineClass<Mat4>(sn, Mat4::metaTableName());

  mat4.metamethod<lMat4Mul>("__mul");
  mat4.method<lMat4Copy>("copy");
  mat4.method<lMat4Transpose>("transpose");

  mat4.raw_static_function<lMat4New>("new");
  mat4.static_function<lMat4Identity>("identity");
}
} // namespace sinen
