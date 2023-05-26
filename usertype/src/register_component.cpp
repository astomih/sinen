#include "actor/actor.hpp"
#include "component/draw3d_component.hpp"
#include <component/move_component.hpp>
#include <component/rigidbody_component.hpp>

#include "register_script.hpp"

namespace sinen {
void register_component(sol::state &lua) {
  lua["actor"] = []() -> actor { return actor(); };
  lua["cconvert_draw3d"] = [](component *c) -> draw3d_component * {
    return dynamic_cast<draw3d_component *>(c);
  };

  {
    auto v = lua.new_usertype<actor>("", sol::no_construction());
    v["get_position"] = &actor::get_position;
    v["set_position"] = &actor::set_position;
    v["get_rotation"] = &actor::get_rotation;
    v["set_rotation"] = &actor::set_rotation;
    v["get_scale"] = &actor::get_scale;
    v["set_scale"] = &actor::set_scale;
    v["has_component"] = [](actor &a, std::string_view str) {
      return a.has_component(str);
    };
    v["get_component"] = &actor::get_component;
    v["add_component"] = [](actor &a, std::string_view str) {
      return a.add_component(str);
    };
  };
  { auto v = lua.new_usertype<component>("", sol::no_construction()); }
  {
    auto v = lua.new_usertype<draw3d_component>("", sol::no_construction());
    v["set_texture"] = &draw3d_component::set_texture;
    v["set_vertex_name"] = &draw3d_component::set_vertex_name;
  }
  { auto v = lua.new_usertype<move_component>("", sol::no_construction()); }
  {
    auto v = lua.new_usertype<rigidbody_component>("", sol::no_construction());
  }
}
} // namespace sinen