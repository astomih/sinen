#include "luaapi.hpp"
#include <math/geometry/pivot.hpp>

namespace sinen {
void registerPivot(lua_State *L) {
  pushSnNamed(L, "Pivot");
  Binding::registerInteger(L, "TopLeft",
                         static_cast<lua_Integer>(Pivot::TopLeft));
  Binding::registerInteger(L, "TopCenter",
                         static_cast<lua_Integer>(Pivot::TopCenter));
  Binding::registerInteger(L, "TopRight",
                         static_cast<lua_Integer>(Pivot::TopRight));
  Binding::registerInteger(L, "Left", static_cast<lua_Integer>(Pivot::Left));
  Binding::registerInteger(L, "Center", static_cast<lua_Integer>(Pivot::Center));
  Binding::registerInteger(L, "Right", static_cast<lua_Integer>(Pivot::Right));
  Binding::registerInteger(L, "BottomLeft",
                         static_cast<lua_Integer>(Pivot::BottomLeft));
  Binding::registerInteger(L, "BottomCenter",
                         static_cast<lua_Integer>(Pivot::BottomCenter));
  Binding::registerInteger(L, "BottomRight",
                         static_cast<lua_Integer>(Pivot::BottomRight));
  lua_pop(L, 1);
}
} // namespace sinen
