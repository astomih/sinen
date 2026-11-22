#include "luaapi.hpp"
#include <math/quaternion.hpp>
#include <math/vec3.hpp>

namespace sinen {
void registerVec3(lua_State *L) {
  engineClass<Vec3>(L, Vec3::metaTableName())
      .static_function("new",
                       luau::overload([] { return Vec3(0.0f); },
                                      [](float value) { return Vec3(value); },
                                      [](float x, float y, float z) {
                                        return Vec3(x, y, z);
                                      }))
      .field("x", &Vec3::x)
      .field("y", &Vec3::y)
      .field("z", &Vec3::z)
      .metamethod("__add", [](const Vec3 &a, const Vec3 &b) { return a + b; })
      .metamethod("__sub", [](const Vec3 &a, const Vec3 &b) { return a - b; })
      .metamethod("__mul", [](const Vec3 &a, const Vec3 &b) { return a * b; })
      .metamethod("__div", [](const Vec3 &a, const Vec3 &b) { return a / b; })
      .metamethod("__tostring", [](const Vec3 &v) { return v.tableString(); })
      .method("copy", [](const Vec3 &v) { return v; })
      .method("length", [](const Vec3 &v) { return v.length(); })
      .method("forward",
              [](const Vec3 &v, const Vec3 &rotation) {
                return Vec3::transform(v, Quat::fromEuler(rotation));
              })
      .method("normalize",
              [](Vec3 v) {
                v.normalize();
                return v;
              })
      .method("dot",
              [](const Vec3 &a, const Vec3 &b) { return Vec3::dot(a, b); })
      .method("cross",
              [](const Vec3 &a, const Vec3 &b) { return Vec3::cross(a, b); })
      .method("reflect",
              [](const Vec3 &a, const Vec3 &b) { return Vec3::reflect(a, b); })
      .method("lerp", [](const Vec3 &a, const Vec3 &b, float value) {
        return Vec3::lerp(a, b, value);
      });
}
} // namespace sinen
