#include "luaapi.hpp"
#include <math/transform/transform.hpp>

namespace sinen {
void registerTransform(luau::table &sn) {
  engineClass<Transform>(sn, Transform::metaTableName())
      .constructor<>()
      .reference_field<&Transform::position>("position")
      .reference_field<&Transform::rotation>("rotation")
      .reference_field<&Transform::scale>("scale")
      .metamethod<&Transform::tableString>("__tostring")
      .method<&Transform::getWorldMatrix>("getWorldMatrix");
}
} // namespace sinen
