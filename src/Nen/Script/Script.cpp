#include "../Texture/texture_system.hpp"
#include <Nen.hpp>
#include <Nen/DrawObject/draw_object_wrapper.hpp>
#include <functional>
#include <sol/sol.hpp>

namespace nen {
draw2d::draw2d()
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(10.f, 10.f)) {}
draw2d::draw2d(texture texture_handle)
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(10.f, 10.f)),
      texture_handle(texture_handle) {}
draw3d::draw3d()
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(10.f, 10.f, 10.f)) {}
draw3d::draw3d(texture texture_handle)
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(10.f, 10.f, 10.f)), texture_handle(texture_handle) {}
void draw2d::draw() {
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::NegUnitZ, rotation);
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  matrix4 ts = matrix4::Identity;
  ts.mat[0][0] = static_cast<float>(texture(texture_handle).GetWidth());
  ts.mat[1][1] = static_cast<float>(texture(texture_handle).GetHeight());
  obj->param.world = ts * s * r * t;
  obj->texture_handle = texture_handle.handle;
  matrix4 viewproj = matrix4::Identity;

  auto windowsize = get_window().Size();
  viewproj.mat[0][0] = 1.f / windowsize.x;
  viewproj.mat[1][1] = 1.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::Identity;
  obj->vertexIndex = "SPRITE";
  get_renderer().draw2d(obj);
}
void draw3d::draw() {
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  t.mat[3][2] = position.z;
  quaternion q(vector3::NegUnitZ, rotation.z);
  q = quaternion::Concatenate(q, quaternion(vector3::UnitY, rotation.y));
  q = quaternion::Concatenate(q, quaternion(vector3::UnitX, rotation.x));
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  s.mat[2][2] = scale.z;
  obj->param.world = s * r * t;
  obj->texture_handle = texture_handle.handle;
  obj->param.proj = get_renderer().GetProjectionMatrix();
  obj->param.view = get_renderer().GetViewMatrix();
  obj->vertexIndex = "SPRITE";
  get_renderer().draw3d(obj);
}
bool script_system::initialize() {
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  lua["create_texture"] = [&]() -> handle_t {
    return get_texture_system().create();
  };
  lua["create_font"] = [&]() -> handle_t {
    return get_font_system().create_font();
  };
  lua["require"] = [&](const std::string &str) -> sol::object {
    return lua.require_file(str, nen::asset_reader::ConvertFilePath(
                                     str + ".lua", nen::asset_type::Script));
  };
  lua["script"] = [&](const std::string &str) {
    lua.script_file(nen::asset_reader::ConvertFilePath(
        str + ".lua", nen::asset_type::Script));
  };

  lua["texture"] = [&]() -> texture {
    return texture(get_texture_system().create());
  };
  lua["draw2d"] = [&]() -> draw2d { return draw2d(); };
  lua["draw2d"] = [&](texture t) -> draw2d { return draw2d(t); };
  lua["draw3d"] = [&]() -> draw3d { return draw3d(); };
  lua["draw3d"] = [&](texture t) -> draw3d { return draw3d(t); };
  lua["vector3"] = [&](float x, float y, float z) -> vector3 {
    return vector3(x, y, z);
  };
  lua["vector2"] = [&](float x, float y) -> vector2 { return vector2(x, y); };
  lua["quaternion"] = [&](sol::this_state s) -> quaternion {
    return quaternion();
  };
  lua["color"] = [&](float r, float g, float b, float a) -> color {
    return color(r, g, b, a);
  };

  return true;
}

void script_system::DoScript(std::string_view fileName) {
  lua.script_file(asset_reader::ConvertFilePath(fileName, asset_type::Script));
}

} // namespace nen
