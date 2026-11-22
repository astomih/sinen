#include "luaapi.hpp"
#include <math/geometry/pivot.hpp>

namespace sinen {
void registerPivot(luau::table &sn) {
  auto pivot = sn.get_or_create_table("Pivot");
  pivot.value("TopLeft", Pivot::TopLeft);
  pivot.value("TopCenter", Pivot::TopCenter);
  pivot.value("TopRight", Pivot::TopRight);
  pivot.value("Left", Pivot::Left);
  pivot.value("Center", Pivot::Center);
  pivot.value("Right", Pivot::Right);
  pivot.value("BottomLeft", Pivot::BottomLeft);
  pivot.value("BottomCenter", Pivot::BottomCenter);
  pivot.value("BottomRight", Pivot::BottomRight);
}
} // namespace sinen
