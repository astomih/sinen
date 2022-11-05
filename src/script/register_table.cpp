#include "register_script.hpp"

#include "../render/render_system.hpp"
#include "../window/window_system.hpp"
#include <window/window.hpp>


namespace sinen {
void register_table(sol::state &lua) {
  {
    auto v = lua.create_table("window");
    v["name"] = &window::name;
    v["state"] = []() { return static_cast<int>(window::state()); };
    v["size"] = &window::size;
  }
  {
    auto v = lua.create_table("renderer");
    v["set_skybox"] = &renderer::set_skybox;
    v["skybox"] = &renderer::skybox;
    v["clear_color"] = &renderer::clear_color;
    v["set_clear_color"] = &renderer::set_clear_color;
    v["get_graphics_api"] = &renderer::get_graphics_api;
  }
}
} // namespace sinen