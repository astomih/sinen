#include <math/geometry/pivot.hpp>
#include <script/luaapi.hpp>

namespace sinen {
void registerPivot(lua_State *L) {
  pushSnNamed(L, "Pivot");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopLeft));
  lua_setfield(L, -2, "TopLeft");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopCenter));
  lua_setfield(L, -2, "TopCenter");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::TopRight));
  lua_setfield(L, -2, "TopRight");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::Center));
  lua_setfield(L, -2, "Center");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomLeft));
  lua_setfield(L, -2, "BottomLeft");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomCenter));
  lua_setfield(L, -2, "BottomCenter");
  lua_pushinteger(L, static_cast<lua_Integer>(Pivot::BottomRight));
  lua_setfield(L, -2, "BottomRight");
  lua_pop(L, 1);
}
} // namespace sinen
