#include "luaapi.hpp"
#include <math/geometry/ray.hpp>

namespace sinen {
void registerRay(luau::table &sn) {
  engineClass<Ray>(sn, Ray::metaTableName())
      .static_function(
          "new", luau::overload([] { return Ray{}; },
                                [](const Vec3 &origin, const Vec3 &direction) {
                                  return Ray{origin, direction};
                                }))
      .property("origin", [](Ray &ray) { return std::ref(ray.origin); })
      .property("direction", [](Ray &ray) { return std::ref(ray.direction); });
}
} // namespace sinen
