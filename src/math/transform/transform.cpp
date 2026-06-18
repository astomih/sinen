#include <math/transform/transform.hpp>
#include <core/data/table_string.hpp>
#include <math/matrix.hpp>
#include <core/data/table_string.hpp>
#include <math/quaternion.hpp>
#include <core/data/table_string.hpp>

namespace sinen {
Mat4 Transform::getWorldMatrix() const {
  const auto t = Mat4::translate(position);
  const auto r = Mat4::fromQuat(Quat::fromEuler(rotation));
  const auto s = Mat4::scale(scale);
  return t * r * s;
}
} // namespace sinen

namespace sinen {
String Transform::tableString() const {
  TablePair p;
  p.emplace_back("position", position.tableString());
  p.emplace_back("rotation", rotation.tableString());
  p.emplace_back("scale", scale.tableString());
  return convert("sn.Transform", p, true);
}
} // namespace sinen
