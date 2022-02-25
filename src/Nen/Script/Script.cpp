#include <Nen.hpp>
#include <functional>
#include <sol/sol.hpp>

namespace nen {
bool script_system::initialize() {
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  lua["create_texture"] = [&]() -> handle_t {
    return m_manager.get_texture_system().create_texture();
  };
  lua["create_font"] = [&]() -> handle_t {
    return m_manager.get_font_system().create_font();
  };
  lua["require"] = [&](const std::string &str) -> sol::object {
    return lua.require_file(str, nen::asset_reader::ConvertFilePath(
                                     str + ".lua", nen::asset_type::Script));
  };
  lua["script"] = [&](const std::string &str) {
    lua.script_file(nen::asset_reader::ConvertFilePath(
        str + ".lua", nen::asset_type::Script));
  };

  auto draw2d = [](sol::this_state s) -> sol::table {
    sol::state_view lua(s);

    sol::table modules = lua.create_table();
    modules["rect"] = [](const draw2d_object &object) -> int {
      auto obj = std::make_shared<draw_object>();
      matrix4 t = matrix4::Identity;
      t.mat[3][0] = object.position.x;
      t.mat[3][1] = object.position.y;
      quaternion q(vector3::NegUnitZ, object.rotation);
      matrix4 r = matrix4::CreateFromQuaternion(q);
      matrix4 s = matrix4::Identity;
      s.mat[0][0] = object.scale.x;
      s.mat[1][1] = object.scale.y;
      matrix4 ts = matrix4::Identity;
      auto &mTexture = get_texture_system().get_texture(object.texture_handle);
      ts.mat[0][0] = static_cast<float>(mTexture.GetWidth());
      ts.mat[1][1] = static_cast<float>(mTexture.GetHeight());
      obj->param.world = ts * s * r * t;
      obj->texture_handle = object.texture_handle;
      matrix4 viewproj = matrix4::Identity;

      auto windowsize = get_window().Size();
      viewproj.mat[0][0] = 1.f / windowsize.x;
      viewproj.mat[1][1] = 1.f / windowsize.y;
      obj->param.proj = viewproj;
      obj->param.view = matrix4::Identity;
      obj->textureIndex = mTexture.id;
      obj->vertexIndex = "SPRITE";
      get_renderer().draw2d(obj);
      return 2;
    };

    return modules;
  };
  lua.require("draw2d", sol::c_call<decltype(draw2d), draw2d>);
  lua["texture_fill_color"] = [&](handle_t handle, const color &color,
                                  std::string_view ID) {
    m_manager.get_texture_system().get_texture(handle).fill_color(color, ID);
  };

  return true;
}

void script_system::DoScript(std::string_view fileName) {
  lua.script_file(asset_reader::ConvertFilePath(fileName, asset_type::Script));
}

} // namespace nen
