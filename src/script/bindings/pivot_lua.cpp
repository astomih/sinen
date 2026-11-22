#include "luaapi.hpp"
#include <math/geometry/pivot.hpp>

namespace sinen {
void registerPivot(lua_State *L) {
  pushSnNamed(L, "Pivot");
  luau::table(L, -1).value("TopLeft", static_cast<lua_Integer>(Pivot::TopLeft));
  luau::table(L, -1).value("TopCenter",
                           static_cast<lua_Integer>(Pivot::TopCenter));
  luau::table(L, -1).value("TopRight",
                           static_cast<lua_Integer>(Pivot::TopRight));
  luau::table(L, -1).value("Left", static_cast<lua_Integer>(Pivot::Left));
  luau::table(L, -1).value("Center", static_cast<lua_Integer>(Pivot::Center));
  luau::table(L, -1).value("Right", static_cast<lua_Integer>(Pivot::Right));
  luau::table(L, -1).value("BottomLeft",
                           static_cast<lua_Integer>(Pivot::BottomLeft));
  luau::table(L, -1).value("BottomCenter",
                           static_cast<lua_Integer>(Pivot::BottomCenter));
  luau::table(L, -1).value("BottomRight",
                           static_cast<lua_Integer>(Pivot::BottomRight));
  lua_pop(L, 1);
}
} // namespace sinen
