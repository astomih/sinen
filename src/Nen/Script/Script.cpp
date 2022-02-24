#include <Nen.hpp>
#include <functional>
#include <sol/sol.hpp>

namespace nen {
bool script_system::initialize() {
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  lua["add_texture"] = [&]() -> handle_t {
    return m_manager.get_texture_system().add_texture();
  };
  lua["get_texture"] = [&](handle_t handle) -> texture & {
    return m_manager.get_texture_system().get_texture(handle);
  };
  lua["add_font"] = [&]() -> handle_t {
    return m_manager.get_font_system().add_font();
  };
  lua["get_font"] = [&](handle_t handle) -> font & {
    return m_manager.get_font_system().get_font(handle);
  };
  lua["require"] = [&](const std::string &str) -> sol::object {
    return lua.require_file(str, nen::asset_reader::ConvertFilePath(
                                     str + ".lua", nen::asset_type::Script));
  };
  lua["script"] = [&](const std::string &str) {
    lua.script_file(nen::asset_reader::ConvertFilePath(
        str + ".lua", nen::asset_type::Script));
  };
  lua["texture_fill_color"] = [&](handle_t handle, const color &color,
                                  std::string_view ID) {
    m_manager.get_texture_system().get_texture(handle).CreateFromColor(color,
                                                                       ID);
  };

  return true;
}

void script_system::DoScript(std::string_view fileName) {
  lua.script_file(asset_reader::ConvertFilePath(fileName, asset_type::Script));
}

} // namespace nen
