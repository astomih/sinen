#ifdef SINEN_MODULE_RENDERER

#include "luaapi.hpp"
#include <renderer/renderer.hpp>

namespace sinen {
namespace {

int setPointLight(lua_State *L) {
  const auto &position = udValue<Vec3>(L, 2);
  const auto intensity = static_cast<float>(luaL_checknumber(L, 3));
  const auto color = lua_isnoneornil(L, 4) ? Color(1.0f) : udValue<Color>(L, 4);
  udValue<PBRRenderer>(L, 1).setPointLight(position, intensity, color);
  return 0;
}

int setEnvironment(lua_State *L) {
  const auto &lut = udPtr<Texture>(L, 2);
  const auto &irradiance = udPtr<Texture>(L, 3);
  const auto &prefiltered = udPtr<Texture>(L, 4);
  udValue<PBRRenderer>(L, 1).setEnvironment({lut, irradiance, prefiltered});
  return 0;
}

int clearEnvironment(lua_State *L) {
  udValue<PBRRenderer>(L, 1).clearEnvironment();
  return 0;
}

int setLightDirection(lua_State *L) {
  udValue<NPRRenderer>(L, 1).setLightDirection(udValue<Vec3>(L, 2));
  return 0;
}

int setShadowColor(lua_State *L) {
  udValue<NPRRenderer>(L, 1).setShadowColor(udValue<Color>(L, 2));
  return 0;
}

int setBands(lua_State *L) {
  const auto bands = luaL_checkinteger(L, 2);
  luaL_argcheck(L, bands >= 2, 2, "expected at least two shading bands");
  udValue<NPRRenderer>(L, 1).setBands(static_cast<UInt32>(bands));
  return 0;
}

template <class T> int newRenderer(lua_State *L) {
  udNewOwned<T>(L, T{});
  return 1;
}

template <class T> int drawModel(lua_State *L) {
  auto &pass = udPtr<Render3DPass>(L, 2);
  const auto &model = udPtr<Model>(L, 3);
  const auto &transform = udValue<Transform>(L, 4);
  udValue<T>(L, 1).drawModel(*pass, *model, transform);
  return 0;
}

template <class T> void registerRendererMetatable(lua_State *L) {
  luaL_newmetatable(L, T::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "drawModel", drawModel<T>);
}

} // namespace

void registerRenderer(lua_State *L) {
  registerRendererMetatable<PBRRenderer>(L);
  Binding::registerFunction(L, "setPointLight", setPointLight);
  Binding::registerFunction(L, "setEnvironment", setEnvironment);
  Binding::registerFunction(L, "clearEnvironment", clearEnvironment);
  lua_pop(L, 1);

  registerRendererMetatable<NPRRenderer>(L);
  Binding::registerFunction(L, "setLightDirection", setLightDirection);
  Binding::registerFunction(L, "setShadowColor", setShadowColor);
  Binding::registerFunction(L, "setBands", setBands);
  lua_pop(L, 1);

  pushSnNamed(L, "PBRRenderer");
  Binding::registerFunction(L, "new", newRenderer<PBRRenderer>);
  lua_pop(L, 1);

  pushSnNamed(L, "NPRRenderer");
  Binding::registerFunction(L, "new", newRenderer<NPRRenderer>);
  lua_pop(L, 1);
}

} // namespace sinen

#endif
