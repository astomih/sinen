#include "../audio/sound_system.hpp"
#include "../render/render_system.hpp"
#include "../texture/texture_system.hpp"
#include "math/vector3.hpp"
#include "script_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <drawable/drawable_wrapper.hpp>
#include <font/font.hpp>
#include <functional>
#include <input/input.hpp>
#include <instancing/instancing_wrapper.hpp>
#include <io/dstream.hpp>
#include <main/main.hpp>
#include <math/random.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>
#include <window/window.hpp>

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
  impl->state["draw2d"] = []() -> draw2d { return draw2d(); };
  impl->state["draw2d"] = [](texture t) -> draw2d { return draw2d(t); };
  impl->state["draw3d"] = []() -> draw3d { return draw3d(); };
  impl->state["draw3d"] = [](texture t) -> draw3d { return draw3d(t); };
  impl->state["draw2d_instanced"] = [&](texture t) -> draw2d_instancing {
    return draw2d_instancing(t);
  };
  impl->state["draw3d_instanced"] = [](texture t) -> draw3d_instancing {
    return draw3d_instancing(t);
  };
  impl->state["vector3"] = [](float x, float y, float z) -> vector3 {
    return vector3(x, y, z);
  };
  impl->state["vector2"] = [](float x, float y) -> vector2 {
    return vector2(x, y);
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
    auto v = impl->state.new_usertype<color>("", sol::no_construction());
    v["r"] = &color::r;
    v["g"] = &color::g;
    v["b"] = &color::b;
    v["a"] = &color::a;
  }
  {
    auto v = impl->state.new_usertype<draw2d>("", sol::no_construction());
    v["draw"] = &draw2d::draw;
    v["position"] = &draw2d::position;
    v["rotation"] = &draw2d::rotation;
    v["scale"] = &draw2d::scale;
    v["texture"] = &draw2d::texture_handle;
    v["vertex_name"] = &draw2d::vertex_name;
  }
  {
    auto v = impl->state.new_usertype<draw3d>("", sol::no_construction());
    v["position"] = &draw3d::position;
    v["rotation"] = &draw3d::rotation;
    v["scale"] = &draw3d::scale;
    v["texture"] = &draw3d::texture_handle;
    v["draw"] = &draw3d::draw;
    v["vertex_name"] = &draw3d::vertex_name;
    v["is_draw_depth"] = &draw3d::is_draw_depth;
  }
  {
    auto v =
        impl->state.new_usertype<draw2d_instancing>("", sol::no_construction());
    v["draw"] = &draw2d_instancing::draw;
    v["add"] = &draw2d_instancing::add;
    v["clear"] = &draw2d_instancing::clear;
    v["texture"] = &draw2d_instancing::texture_handle;
    v["vertex_name"] = &draw2d_instancing::vertex_name;
  }
  {
    auto v =
        impl->state.new_usertype<draw3d_instancing>("", sol::no_construction());
    v["draw"] = &draw3d_instancing::draw;
    v["add"] = &draw3d_instancing::add;
    v["clear"] = &draw3d_instancing::clear;
    v["texture"] = &draw3d_instancing::texture_handle;
    v["vertex_name"] = &draw3d_instancing::vertex_name;
    v["is_draw_depth"] = &draw3d_instancing::is_draw_depth;
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
    v["aabb"] = &model::m_aabb;
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
  {
    auto &v = impl->state;
    v["keyA"] = (int)key_code::A;
    v["keyB"] = (int)key_code::B;
    v["keyC"] = (int)key_code::C;
    v["keyD"] = (int)key_code::D;
    v["keyE"] = (int)key_code::E;
    v["keyF"] = (int)key_code::F;
    v["keyG"] = (int)key_code::G;
    v["keyH"] = (int)key_code::H;
    v["keyI"] = (int)key_code::I;
    v["keyJ"] = (int)key_code::J;
    v["keyK"] = (int)key_code::K;
    v["keyL"] = (int)key_code::L;
    v["keyM"] = (int)key_code::M;
    v["keyN"] = (int)key_code::N;
    v["keyO"] = (int)key_code::O;
    v["keyP"] = (int)key_code::P;
    v["keyQ"] = (int)key_code::Q;
    v["keyR"] = (int)key_code::R;
    v["keyS"] = (int)key_code::S;
    v["keyT"] = (int)key_code::T;
    v["keyU"] = (int)key_code::U;
    v["keyV"] = (int)key_code::V;
    v["keyW"] = (int)key_code::W;
    v["keyX"] = (int)key_code::X;
    v["keyY"] = (int)key_code::Y;
    v["keyZ"] = (int)key_code::Z;
    v["key0"] = (int)key_code::Key0;
    v["key1"] = (int)key_code::Key1;
    v["key2"] = (int)key_code::Key2;
    v["key3"] = (int)key_code::Key3;
    v["key4"] = (int)key_code::Key4;
    v["key5"] = (int)key_code::Key5;
    v["key6"] = (int)key_code::Key6;
    v["key7"] = (int)key_code::Key7;
    v["key8"] = (int)key_code::Key8;
    v["key9"] = (int)key_code::Key9;
    v["keyF1"] = (int)key_code::F1;
    v["keyF2"] = (int)key_code::F2;
    v["keyF3"] = (int)key_code::F3;
    v["keyF4"] = (int)key_code::F4;
    v["keyF5"] = (int)key_code::F5;
    v["keyF6"] = (int)key_code::F6;
    v["keyF7"] = (int)key_code::F7;
    v["keyF8"] = (int)key_code::F8;
    v["keyF9"] = (int)key_code::F9;
    v["keyF10"] = (int)key_code::F10;
    v["keyF11"] = (int)key_code::F11;
    v["keyF12"] = (int)key_code::F12;
    v["keyUP"] = (int)key_code::UP;
    v["keyDOWN"] = (int)key_code::DOWN;
    v["keyLEFT"] = (int)key_code::LEFT;
    v["keyRIGHT"] = (int)key_code::RIGHT;
    v["keyESCAPE"] = (int)key_code::ESCAPE;
    v["keySPACE"] = (int)key_code::SPACE;
    v["keyENTER"] = (int)key_code::KP_ENTER;
    v["keyBACKSPACE"] = (int)key_code::BACKSPACE;
    v["keyTAB"] = (int)key_code::TAB;
    v["keyLSHIFT"] = (int)key_code::LSHIFT;
    v["keyRSHIFT"] = (int)key_code::RSHIFT;
    v["keyLCTRL"] = (int)key_code::LCTRL;
    v["keyRCTRL"] = (int)key_code::RCTRL;
    v["keyALT"] = (int)key_code::ALTERASE;
    v["keyLCTRL"] = (int)key_code::LCTRL;
    v["keyRCTRL"] = (int)key_code::RCTRL;
  }
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
  {
    auto v = impl->state.create_table("window");
    v["name"] = &window::name;
    v["state"] = []() { return static_cast<int>(window::state()); };
    v["size"] = &window::size;
  }
  {
    auto v = impl->state.create_table("renderer");
    v["set_skybox"] = &renderer::set_skybox;
    v["skybox"] = &renderer::skybox;
    v["clear_color"] = &renderer::clear_color;
    v["set_clear_color"] = &renderer::set_clear_color;
    v["get_graphics_api"] = &renderer::get_graphics_api;
  }

#endif
  return true;
}

void script_system::do_script(std::string_view fileName) {
  impl->state.script(dstream::open_as_string(asset_type::Script, fileName));
}

void script_system::shutdown() { impl->state.collect_gc(); }

void script::do_script(std::string_view fileName) {
  script_system::do_script(fileName);
}

} // namespace sinen
