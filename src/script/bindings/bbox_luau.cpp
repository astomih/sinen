#include "luaapi.hpp"
#include <math/geometry/bbox.hpp>

namespace sinen {
void registerAABB(luau::table &sn) {
  engineClass<AABB>(sn, AABB::metaTableName())
      .constructor<>()
      .reference_field<&AABB::min>("min")
      .reference_field<&AABB::max>("max")
      .method<&AABB::updateWorld>("updateWorld")
      .method<&AABB::intersectsAABB>("intersectsAABB");
}
} // namespace sinen
