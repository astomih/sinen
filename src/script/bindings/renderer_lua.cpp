#ifdef SINEN_MODULE_RENDERER

#include "luaapi.hpp"
#include <renderer/renderer.hpp>

#include <type_traits>

namespace sinen {
namespace {

template <class T> int setLight(lua_State *L) {
  auto &renderer = udValue<T>(L, 1);
  if (luaLTestudata2(L, 2, DirectionalLight::metaTableName())) {
    renderer.setLight(udValue<DirectionalLight>(L, 2));
  } else if (luaLTestudata2(L, 2, PointLight::metaTableName())) {
    renderer.setLight(udValue<PointLight>(L, 2));
  } else if (luaLTestudata2(L, 2, SpotLight::metaTableName())) {
    renderer.setLight(udValue<SpotLight>(L, 2));
  } else {
    luaL_argerror(L, 2, "expected DirectionalLight, PointLight, or SpotLight");
  }
  return 0;
}

Vec3 optionalVec3(lua_State *L, int index, const Vec3 &fallback) {
  return lua_isnoneornil(L, index) ? fallback : udValue<Vec3>(L, index);
}

Color optionalColor(lua_State *L, int index) {
  return lua_isnoneornil(L, index) ? Color(1.0f) : udValue<Color>(L, index);
}

int newDirectionalLight(lua_State *L) {
  udNewOwned<DirectionalLight>(
      L, DirectionalLight(optionalVec3(L, 1, Vec3(0, 0, -1)),
                          static_cast<float>(luaL_optnumber(L, 2, 1.0)),
                          optionalColor(L, 3)));
  return 1;
}

int newPointLight(lua_State *L) {
  udNewOwned<PointLight>(
      L, PointLight(optionalVec3(L, 1, Vec3(0)),
                    static_cast<float>(luaL_optnumber(L, 2, 4.0)),
                    optionalColor(L, 3)));
  return 1;
}

int newSpotLight(lua_State *L) {
  udNewOwned<SpotLight>(
      L,
      SpotLight(optionalVec3(L, 1, Vec3(0)), optionalVec3(L, 2, Vec3(0, 0, -1)),
                static_cast<float>(luaL_optnumber(L, 3, 20.0)),
                static_cast<float>(luaL_optnumber(L, 4, 35.0)),
                static_cast<float>(luaL_optnumber(L, 5, 4.0)),
                optionalColor(L, 6)));
  return 1;
}

template <class T> int setPosition(lua_State *L) {
  udValue<T>(L, 1).setPosition(udValue<Vec3>(L, 2));
  return 0;
}

template <class T> int getPosition(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<T>(L, 1).getPosition());
  return 1;
}

template <class T> int setDirection(lua_State *L) {
  udValue<T>(L, 1).setDirection(udValue<Vec3>(L, 2));
  return 0;
}

template <class T> int getDirection(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<T>(L, 1).getDirection());
  return 1;
}

template <class T> int setIntensity(lua_State *L) {
  udValue<T>(L, 1).setIntensity(static_cast<float>(luaL_checknumber(L, 2)));
  return 0;
}

template <class T> int getIntensity(lua_State *L) {
  lua_pushnumber(L, udValue<T>(L, 1).getIntensity());
  return 1;
}

template <class T> int setColor(lua_State *L) {
  udValue<T>(L, 1).setColor(udValue<Color>(L, 2));
  return 0;
}

template <class T> int getColor(lua_State *L) {
  udNewOwned<Color>(L, udValue<T>(L, 1).getColor());
  return 1;
}

int setConeAngles(lua_State *L) {
  const auto inner = static_cast<float>(luaL_checknumber(L, 2));
  const auto outer = static_cast<float>(luaL_checknumber(L, 3));
  udValue<SpotLight>(L, 1).setConeAngles(inner, outer);
  return 0;
}

int getConeAngles(lua_State *L) {
  const auto &light = udValue<SpotLight>(L, 1);
  lua_pushnumber(L, light.getInnerAngle());
  lua_pushnumber(L, light.getOuterAngle());
  return 2;
}

template <class T>
void registerLight(lua_State *L, const char *name, lua_CFunction constructor) {
  luaL_newmetatable(L, T::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "setIntensity", setIntensity<T>);
  Binding::registerFunction(L, "getIntensity", getIntensity<T>);
  Binding::registerFunction(L, "setColor", setColor<T>);
  Binding::registerFunction(L, "getColor", getColor<T>);
  if constexpr (!std::is_same_v<T, DirectionalLight>) {
    Binding::registerFunction(L, "setPosition", setPosition<T>);
    Binding::registerFunction(L, "getPosition", getPosition<T>);
  }
  if constexpr (!std::is_same_v<T, PointLight>) {
    Binding::registerFunction(L, "setDirection", setDirection<T>);
    Binding::registerFunction(L, "getDirection", getDirection<T>);
  }
  if constexpr (std::is_same_v<T, SpotLight>) {
    Binding::registerFunction(L, "setConeAngles", setConeAngles);
    Binding::registerFunction(L, "getConeAngles", getConeAngles);
  }
  lua_pop(L, 1);
  pushSnNamed(L, name);
  Binding::registerFunction(L, "new", constructor);
  lua_pop(L, 1);
}

int setPointLight(lua_State *L) {
  const auto &position = udValue<Vec3>(L, 2);
  const auto intensity = static_cast<float>(luaL_checknumber(L, 3));
  const auto color = optionalColor(L, 4);
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
  Binding::registerFunction(L, "setLight", setLight<T>);
  Binding::registerFunction(L, "drawModel", drawModel<T>);
}

} // namespace

void registerRenderer(lua_State *L) {
  registerLight<DirectionalLight>(L, "DirectionalLight", newDirectionalLight);
  registerLight<PointLight>(L, "PointLight", newPointLight);
  registerLight<SpotLight>(L, "SpotLight", newSpotLight);

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
