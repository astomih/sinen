#include "../audio/sound_system.hpp"
#include "../render/render_system.hpp"
#include "../texture/texture_system.hpp"
#include "math/vector3.hpp"
#include "script_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <functional>
#include <input/input.hpp>
#include <io/dstream.hpp>
#include <main/main.hpp>
#include <math/point2.hpp>
#include <math/random.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>

#include "register_script.hpp"

namespace sinen {
class script_system::implement {
public:
  sol::state state;
};
std::unique_ptr<script_system::implement> script_system::impl = nullptr;
void *script::get_state() { return script_system::get_state(); }
void *script_system::get_state() { return (void *)&impl->state; }
bool script_system::initialize() {
  impl = std::make_unique<implement>();
#ifndef sinen_NO_USE_SCRIPT
  impl->state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                             sol::lib::bit32, sol::lib::io, sol::lib::os,
                             sol::lib::string, sol::lib::debug,
                             sol::lib::table);
  impl->state["require"] = [&](const std::string &str) -> sol::object {
    return impl->state.require_script(
        str, dstream::open_as_string(asset_type::Script, str + ".lua"));
  };
  impl->state["texture"] = []() -> texture { return texture(); };
  impl->state["font"] = []() -> font { return font(); };
  impl->state["DEFAULT_FONT"] = "mplus/mplus-1p-medium.ttf";
  impl->state["vector3"] = [](float x, float y, float z) -> vector3 {
    return vector3(x, y, z);
  };
  impl->state["vector2"] = [](float x, float y) -> vector2 {
    return vector2(x, y);
  };
  impl->state["point2i"] = [](int x, int y) -> point2i {
    return point2i(x, y);
  };
  impl->state["quaternion"] = [](sol::this_state s) -> quaternion {
    return quaternion();
  };
  impl->state["color"] = [](float r, float g, float b, float a) -> color {
    return color(r, g, b, a);
  };
  impl->state["model"] = []() -> model { return model(); };
  impl->state["music"] = []() -> music { return music(); };
  impl->state["sound"] = []() -> sound { return sound(); };
  impl->state["set_skybox_texture"] = [&](texture tex) -> void {
    render_system::set_skybox_texture(tex);
  };
  impl->state["aabb"] = []() -> aabb { return aabb(); };
  {
    auto v = impl->state.new_usertype<vector3>("", sol::no_construction());
    v["x"] = &vector3::x;
    v["y"] = &vector3::y;
    v["z"] = &vector3::z;
    v["add"] = &vector3::add;
    v["sub"] = &vector3::sub;
    v["mul"] = &vector3::mul;
    v["div"] = &vector3::div;
    v["copy"] = &vector3::copy;
    v["forward"] = [](const vector3 v, const vector3 rotation) -> vector3 {
      quaternion q;
      q = quaternion::concatenate(
          q, quaternion(vector3::unit_z, math::to_radians(rotation.z)));
      q = quaternion::concatenate(
          q, quaternion(vector3::unit_y, math::to_radians(rotation.y)));
      q = quaternion::concatenate(
          q, quaternion(vector3::unit_x, math::to_radians(rotation.x)));
      return vector3::transform(v, q);
    };
  }
  {
    auto v = impl->state.new_usertype<vector2>("", sol::no_construction());
    v["x"] = &vector2::x;
    v["y"] = &vector2::y;
    v["add"] = &vector2::add;
    v["sub"] = &vector2::sub;
    v["mul"] = &vector2::mul;
    v["div"] = &vector2::div;
  }
  {
    auto v = impl->state.new_usertype<point2i>("", sol::no_construction());
    v["x"] = &point2i::x;
    v["y"] = &point2i::y;
  }
  {
    auto v = impl->state.new_usertype<color>("", sol::no_construction());
    v["r"] = &color::r;
    v["g"] = &color::g;
    v["b"] = &color::b;
    v["a"] = &color::a;
  }
  {
    auto v = impl->state.new_usertype<texture>("", sol::no_construction());
    v["fill_color"] = &texture::fill_color;
    v["blend_color"] = &texture::blend_color;
    v["copy"] = &texture::copy;
    v["load"] = &texture::load;
    v["size"] = &texture::size;
  }
  {
    auto v = impl->state.new_usertype<font>("", sol::no_construction());
    v["load"] = &font::load;
    v["render_text"] = &font::render_text;
  }
  {
    auto v =
        impl->state.new_usertype<keyboard_state>("", sol::no_construction());
    v["is_key_down"] = &keyboard_state::is_key_down;
    v["key_state"] = &keyboard_state::get_key_state;
  }
  {
    auto v = impl->state.new_usertype<mouse_state>("", sol::no_construction());
    v["button_state"] = &mouse_state::get_button_state;
    v["is_button_down"] = &mouse_state::is_button_down;
    v["position"] = &mouse_state::get_position;
    v["set_position"] = &mouse_state::set_position;
    v["scroll_wheel"] = &mouse_state::get_scroll_wheel;
    v["hide_cursor"] = &mouse_state::hide_cursor;
  }
  {
    auto v = impl->state.new_usertype<music>("", sol::no_construction());
    v["load"] = &music::load;
    v["play"] = &music::play;
    v["set_volume"] = &music::set_volume;
  }
  {
    auto v = impl->state.new_usertype<sound>("", sol::no_construction());
    v["load"] = &sound::load;
    v["play"] = &sound::play;
    v["set_volume"] = &sound::set_volume;
    v["set_pitch"] = &sound::set_pitch;
    v["set_listener"] = &sound::set_listener;
    v["set_position"] = &sound::set_position;
  }
  {
    auto v = impl->state.create_table("camera");
    v["lookat"] = &camera::lookat;
    v["perspective"] = &camera::perspective;
    v["orthographic"] = &camera::orthographic;
    v["position"] = &camera::position;
    v["target"] = &camera::target;
    v["up"] = &camera::up;
  }
  impl->state["change_scene"] = [&](const std::string &str) {
    main::change_scene(str);
  };
  {
    auto v = impl->state.new_usertype<model>("", sol::no_construction());
    v["aabb"] = &model::local_aabb;
    v["load"] = &model::load;
  }
  {
    auto v = impl->state.new_usertype<aabb>("", sol::no_construction());
    v["min"] = &aabb::min;
    v["max"] = &aabb::max;
    v["intersects_aabb"] = &aabb::intersects_aabb;
  }
  {
    auto v = impl->state.new_usertype<random>("", sol::no_construction());
    v["get_int_range"] = random::get_int_range;
    v["get_float_range"] = random::get_float_range;
  }
  register_drawable(impl->state);
  register_keycode(impl->state);
  register_table(impl->state);
  {
    auto &v = impl->state;
    v["mouseLEFT"] = (int)mouse_code::LEFT;
    v["mouseRIGHT"] = (int)mouse_code::RIGHT;
    v["mouseMIDDLE"] = (int)mouse_code::MIDDLE;
    v["mouseX1"] = (int)mouse_code::X1;
    v["mouseX2"] = (int)mouse_code::X2;
  }
  {
    auto &v = impl->state;
    v["buttonNONE"] = (int)button_state::None;
    v["buttonPRESSED"] = (int)button_state::Pressed;
    v["buttonRELEASED"] = (int)button_state::Released;
    v["buttonHELD"] = (int)button_state::Held;
  }
  register_graph(impl->state);

#endif
  return true;
}

void script_system::do_script(std::string_view fileName) {
  impl->state.script(dstream::open_as_string(asset_type::Script, fileName));
}

void script_system::shutdown() { impl->state.collect_gc(); }

void *script_system::new_table(std::string_view table_name) {
  auto *table = new sol::table(impl->state.create_table(table_name));
  return (void *)table;
}

void script_system::register_function(std::string_view name,
                                      std::function<void()> function,
                                      void *table) {
  if (table) {
    auto *t = (sol::table *)table;
    (*t)[name] = function;
    return;
  }
  impl->state[name] = function;
}

void script::do_script(std::string_view fileName) {
  script_system::do_script(fileName);
}

script::table_handler script::new_table(std::string_view table_name) {
  return script_system::new_table(table_name);
}

void script::register_function(std::string_view name,
                               std::function<void()> function,
                               table_handler handler) {
  script_system::register_function(name, function, handler);
}

} // namespace sinen
