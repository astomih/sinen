#ifdef SINEN_MODULE_RENDERER

#include "luaapi.hpp"
#include <renderer/renderer.hpp>

#include <type_traits>

namespace sinen {
namespace {

template <class T, auto Set = &T::setLight> int setLight(lua_State *L) {
  auto &renderer = udValue<T>(L, 1);
  if (luaLTestudata2(L, 2, DirectionalLight::metaTableName())) {
    (renderer.*Set)(udValue<DirectionalLight>(L, 2));
  } else if (luaLTestudata2(L, 2, PointLight::metaTableName())) {
    (renderer.*Set)(udValue<PointLight>(L, 2));
  } else if (luaLTestudata2(L, 2, SpotLight::metaTableName())) {
    (renderer.*Set)(udValue<SpotLight>(L, 2));
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

int clearFillLight(lua_State *L) {
  udValue<PBRRenderer>(L, 1).clearFillLight();
  return 0;
}

int setExposure(lua_State *L) {
  udValue<PBRRenderer>(L, 1).setExposure(
      static_cast<float>(luaL_checknumber(L, 2)));
  return 0;
}

int clearEnvironment(lua_State *L) {
  udValue<PBRRenderer>(L, 1).clearEnvironment();
  return 0;
}

int addGIModel(lua_State *L) {
  udValue<GlobalIllumination>(L, 1).addModel(
      *udPtr<Model>(L, 2), udValue<Transform>(L, 3), optionalColor(L, 4),
      lua_isnoneornil(L, 5) ? Color(0.0f) : udValue<Color>(L, 5));
  return 0;
}
int bakeGI(lua_State *L) {
  UInt32 values[5];
  const int defaults[] = {8, 6, 8, 512, 3};
  const int limits[] = {32, 32, 32, 65536, 8};
  for (int i = 0; i < 5; ++i) {
    const int value = luaL_optinteger(L, i + 4, defaults[i]);
    luaL_argcheck(L,
                  value >= (i < 3    ? 2
                            : i == 3 ? 32
                                     : 1) &&
                      value <= limits[i],
                  i + 4, "GI bake parameter out of range");
    values[i] = static_cast<UInt32>(value);
  }
  lua_pushboolean(L, udValue<GlobalIllumination>(L, 1).bake(
                         udValue<Vec3>(L, 2), udValue<Vec3>(L, 3), values[0],
                         values[1], values[2], values[3], values[4]));
  return 1;
}
int setGISky(lua_State *L) {
  udValue<GlobalIllumination>(L, 1).setSkyColor(udValue<Color>(L, 2));
  return 0;
}
int clearGI(lua_State *L) {
  udValue<GlobalIllumination>(L, 1).clear();
  return 0;
}
int clearGILight(lua_State *L) {
  udValue<GlobalIllumination>(L, 1).clearLight();
  return 0;
}
int isGIBaked(lua_State *L) {
  lua_pushboolean(L, udValue<GlobalIllumination>(L, 1).isBaked());
  return 1;
}
int sampleGI(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<GlobalIllumination>(L, 1).sampleIrradiance(
                          udValue<Vec3>(L, 2), udValue<Vec3>(L, 3)));
  return 1;
}
int setGI(lua_State *L) {
  udValue<PBRRenderer>(L, 1).setGlobalIllumination(
      udValue<GlobalIllumination>(L, 2),
      static_cast<float>(luaL_optnumber(L, 3, 1.0)));
  return 0;
}
int clearRendererGI(lua_State *L) {
  udValue<PBRRenderer>(L, 1).clearGlobalIllumination();
  return 0;
}

int initializeRealtimeGI(lua_State *L) {
  UInt32 counts[4];
  const int defaults[] = {8, 6, 8, 2};
  for (int i = 0; i < 4; ++i) {
    const int value = luaL_optinteger(L, i + 4, defaults[i]);
    luaL_argcheck(L, value >= (i < 3 ? 2 : 1) && value <= (i < 3 ? 32 : 4),
                  i + 4, "real-time GI parameter out of range");
    counts[i] = static_cast<UInt32>(value);
  }
  lua_pushboolean(L, udValue<GlobalIllumination>(L, 1).initializeRealtime(
                         udValue<Vec3>(L, 2), udValue<Vec3>(L, 3), counts[0],
                         counts[1], counts[2], counts[3]));
  return 1;
}
int beginGIFrame(lua_State *L) {
  udValue<GlobalIllumination>(L, 1).beginFrame();
  return 0;
}
int updateGI(lua_State *L) {
  lua_pushboolean(L, udValue<GlobalIllumination>(L, 1).update(
                         static_cast<float>(luaL_optnumber(L, 2, 0.85))));
  return 1;
}
int isRealtimeGI(lua_State *L) {
  lua_pushboolean(L, udValue<GlobalIllumination>(L, 1).isRealtime());
  return 1;
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
  Binding::registerFunction(L, "setFillLight",
                            setLight<PBRRenderer, &PBRRenderer::setFillLight>);
  Binding::registerFunction(L, "clearFillLight", clearFillLight);
  Binding::registerFunction(L, "setExposure", setExposure);
  Binding::registerFunction(L, "setEnvironment", setEnvironment);
  Binding::registerFunction(L, "clearEnvironment", clearEnvironment);
  Binding::registerFunction(L, "setGlobalIllumination", setGI);
  Binding::registerFunction(L, "clearGlobalIllumination", clearRendererGI);
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

  luaL_newmetatable(L, GlobalIllumination::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "addModel", addGIModel);
  Binding::registerFunction(L, "setLight", setLight<GlobalIllumination>);
  Binding::registerFunction(L, "clearLight", clearGILight);
  Binding::registerFunction(L, "setSkyColor", setGISky);
  Binding::registerFunction(L, "bake", bakeGI);
  Binding::registerFunction(L, "initializeRealtime", initializeRealtimeGI);
  Binding::registerFunction(L, "beginFrame", beginGIFrame);
  Binding::registerFunction(L, "update", updateGI);
  Binding::registerFunction(L, "isRealtime", isRealtimeGI);
  Binding::registerFunction(L, "clear", clearGI);
  Binding::registerFunction(L, "isBaked", isGIBaked);
  Binding::registerFunction(L, "sampleIrradiance", sampleGI);
  lua_pop(L, 1);
  pushSnNamed(L, "GlobalIllumination");
  Binding::registerFunction(L, "new", newRenderer<GlobalIllumination>);
  lua_pop(L, 1);
}

} // namespace sinen

#endif
