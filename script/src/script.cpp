#include "../include/script_engine.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <functional>
#include <io/data_stream.hpp>
#include <main/main.hpp>
#include <math/point2.hpp>
#include <math/random.hpp>
#include <math/vector3.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>

#include "register_script.hpp"
#ifdef main
#undef main
#endif

namespace sinen {
bool script_engine::initialize(sol::state &lua) {
#ifndef SINEN_NO_USE_SCRIPT
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                     sol::lib::bit32, sol::lib::io, sol::lib::os,
                     sol::lib::string, sol::lib::debug, sol::lib::table);
  register_generator(lua);
  {
    auto v = lua.new_usertype<vector3>("", sol::no_construction());
    v["x"] = &vector3::x;
    v["y"] = &vector3::y;
    v["z"] = &vector3::z;
    v["add"] = &vector3::add;
    v["sub"] = &vector3::sub;
    v["mul"] = &vector3::mul;
    v["div"] = &vector3::div;
    v["copy"] = &vector3::copy;
    v["length"] = &vector3::length;
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
    v["normalize"] = [](const vector3 &v) { return vector3::normalize(v); };
  }
  {
    auto v = lua.new_usertype<vector2>("", sol::no_construction());
    v["x"] = &vector2::x;
    v["y"] = &vector2::y;
    v["add"] = &vector2::add;
    v["sub"] = &vector2::sub;
    v["mul"] = &vector2::mul;
    v["div"] = &vector2::div;
    v["length"] = &vector2::length;
    v["normalize"] = [](const vector2 &v) { return vector2::normalize(v); };
  }
  {
    auto v = lua.new_usertype<point2i>("", sol::no_construction());
    v["x"] = &point2i::x;
    v["y"] = &point2i::y;
  }
  {
    auto v = lua.new_usertype<color>("", sol::no_construction());
    v["r"] = &color::r;
    v["g"] = &color::g;
    v["b"] = &color::b;
    v["a"] = &color::a;
  }
  {
    auto v = lua.new_usertype<texture>("", sol::no_construction());
    v["fill_color"] = &texture::fill_color;
    v["blend_color"] = &texture::blend_color;
    v["copy"] = &texture::copy;
    v["load"] = &texture::load;
    v["size"] = &texture::size;
  }
  {
    auto v = lua.new_usertype<font>("", sol::no_construction());
    v["load"] = &font::load;
    v["render_text"] = &font::render_text;
    v["resize"] = &font::resize;
  }
  {
    auto v = lua.new_usertype<music>("", sol::no_construction());
    v["load"] = &music::load;
    v["play"] = &music::play;
    v["set_volume"] = &music::set_volume;
  }
  {
    auto v = lua.new_usertype<sound>("", sol::no_construction());
    v["load"] = &sound::load;
    v["play"] = &sound::play;
    v["set_volume"] = &sound::set_volume;
    v["set_pitch"] = &sound::set_pitch;
    v["set_listener"] = &sound::set_listener;
    v["set_position"] = &sound::set_position;
  }
  {
    auto v = lua.new_usertype<camera>("", sol::no_construction());
    v["lookat"] = &camera::lookat;
    v["perspective"] = &camera::perspective;
    v["orthographic"] = &camera::orthographic;
    v["position"] = &camera::position;
    v["target"] = &camera::target;
    v["up"] = &camera::up;
  }
  lua["change_scene"] = [&](const std::string &str) {
    main::change_scene(str);
  };
  {
    auto v = lua.new_usertype<model>("", sol::no_construction());
    v["aabb"] = &model::local_aabb;
    v["load"] = &model::load;
  }
  {
    auto v = lua.new_usertype<aabb>("", sol::no_construction());
    v["min"] = &aabb::min;
    v["max"] = &aabb::max;
    v["update_world"] = &aabb::update_world;
  }
  {
    auto v = lua.new_usertype<random>("", sol::no_construction());
    v["get_int_range"] = random::get_int_range;
    v["get_float_range"] = random::get_float_range;
  }
  register_component(lua);
  register_drawable(lua);
  register_table(lua);
  register_graph(lua);

#endif
  return true;
}

} // namespace sinen
