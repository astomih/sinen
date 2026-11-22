#include "luaapi.hpp"
#include <math/quaternion.hpp>
#include <math/vec3.hpp>

namespace sinen {
void registerVec3(luau::table &sn) {
  engineClass<Vec3>(sn, Vec3::metaTableName())
      .static_function("new",
                       luau::overload([] { return Vec3(0.0f); },
                                      [](float value) { return Vec3(value); },
                                      [](float x, float y, float z) {
                                        return Vec3(x, y, z);
                                      }))
      .field<&Vec3::x>("x")
      .field<&Vec3::y>("y")
      .field<&Vec3::z>("z")
      .metamethod("__add", [](const Vec3 &a, const Vec3 &b) { return a + b; })
      .metamethod("__sub", [](const Vec3 &a, const Vec3 &b) { return a - b; })
      .metamethod("__mul", [](const Vec3 &a, const Vec3 &b) { return a * b; })
      .metamethod("__div", [](const Vec3 &a, const Vec3 &b) { return a / b; })
      .metamethod<&Vec3::tableString>("__tostring")
      .method("copy", [](const Vec3 &v) { return v; })
      .method<&Vec3::length>("length")
      .method("forward",
              [](const Vec3 &v, const Vec3 &rotation) {
                return Vec3::transform(v, Quat::fromEuler(rotation));
              })
      .method("normalize",
              [](Vec3 v) {
                v.normalize();
                return v;
              })
      .method<&Vec3::dot>("dot")
      .method<&Vec3::cross>("cross")
      .method<&Vec3::reflect>("reflect")
      .method<&Vec3::lerp>("lerp");
}
} // namespace sinen
