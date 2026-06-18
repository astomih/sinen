#include <math/vec2.hpp>
#include <core/data/table_string.hpp>

namespace sinen {

String Vec2::tableString() const {
  TablePair p;
  p.emplace_back("x", toStringTrim(x));
  p.emplace_back("y", toStringTrim(y));
  return convert("sn.Vec2", p, false);
}

} // namespace sinen
