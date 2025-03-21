#include "../include/script_engine.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <functional>
#include <io/data_stream.hpp>
#include <math/point2.hpp>
#include <math/random.hpp>
#include <math/vector3.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>
#include <time/timer.hpp>

#include "register_script.hpp"
#include "render/render_pipeline.hpp"
#include "sol/raii.hpp"
#include "sol/types.hpp"
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
    auto v = lua.new_usertype<Vector3>("", sol::no_construction());
    v["x"] = &Vector3::x;
    v["y"] = &Vector3::y;
    v["z"] = &Vector3::z;
    v["__add"] = &Vector3::add;
    v["__sub"] = &Vector3::sub;
    v["__mul"] = &Vector3::mul;
    v["__div"] = &Vector3::div;
    v["copy"] = &Vector3::copy;
    v["length"] = &Vector3::length;
    v["forward"] = [](const Vector3 v, const Vector3 rotation) -> Vector3 {
      Quaternion q;
      q = Quaternion::concatenate(
          q, Quaternion(Vector3::unit_z, Math::to_radians(rotation.z)));
      q = Quaternion::concatenate(
          q, Quaternion(Vector3::unit_y, Math::to_radians(rotation.y)));
      q = Quaternion::concatenate(
          q, Quaternion(Vector3::unit_x, Math::to_radians(rotation.x)));
      return Vector3::transform(v, q);
    };
    v["normalize"] = [](const Vector3 &v) { return Vector3::normalize(v); };
    v["dot"] = &Vector3::dot;
    v["cross"] = &Vector3::cross;
    v["lerp"] = &Vector3::lerp;
    v["reflect"] = &Vector3::reflect;
  }
  {
    auto v = lua.new_usertype<Vector2>("", sol::no_construction());
    v["x"] = &Vector2::x;
    v["y"] = &Vector2::y;
    v["__add"] = &Vector2::add;
    v["__sub"] = &Vector2::sub;
    v["__mul"] = &Vector2::mul;
    v["__div"] = &Vector2::div;
    v["length"] = &Vector2::length;
    v["normalize"] = [](const Vector2 &v) { return Vector2::normalize(v); };
    v["dot"] = &Vector2::dot;
    v["lerp"] = &Vector2::lerp;
    v["reflect"] = &Vector2::reflect;
  }
  {
    auto v = lua.new_usertype<Point2i>("", sol::no_construction());
    v["x"] = &Point2i::x;
    v["y"] = &Point2i::y;
  }
  {
    auto v = lua.new_usertype<Point2f>("", sol::no_construction());
    v["x"] = &Point2f::x;
    v["y"] = &Point2f::y;
  }
  {
    auto v = lua.new_usertype<Color>("", sol::no_construction());
    v["r"] = &Color::r;
    v["g"] = &Color::g;
    v["b"] = &Color::b;
    v["a"] = &Color::a;
  }
  {
    auto v = lua.new_usertype<Texture>("", sol::no_construction());
    v["fill_color"] = &Texture::fill_color;
    v["blend_color"] = &Texture::blend_color;
    v["copy"] = &Texture::copy;
    v["load"] = &Texture::load;
    v["size"] = &Texture::size;
  }
  {
    auto v = lua.new_usertype<RenderTexture>("", sol::no_construction());
    v["create"] = &RenderTexture::create;
  }
  {
    auto v = lua.new_usertype<Font>("", sol::no_construction());
    v["load"] = &Font::load;
    v["render_text"] = &Font::render_text;
    v["resize"] = &Font::resize;
  }
  {
    auto v = lua.new_usertype<Music>("", sol::no_construction());
    v["load"] = &Music::load;
    v["play"] = &Music::play;
    v["set_volume"] = &Music::set_volume;
  }
  {
    auto v = lua.new_usertype<Sound>("", sol::no_construction());
    v["load"] = &Sound::load;
    v["play"] = &Sound::play;
    v["set_volume"] = &Sound::set_volume;
    v["set_pitch"] = &Sound::set_pitch;
    v["set_listener"] = &Sound::set_listener;
    v["set_position"] = &Sound::set_position;
  }
  {
    auto v = lua.new_usertype<Camera>("", sol::no_construction());
    v["lookat"] = &Camera::lookat;
    v["perspective"] = &Camera::perspective;
    v["orthographic"] = &Camera::orthographic;
    v["position"] = &Camera::position;
    v["target"] = &Camera::target;
    v["up"] = &Camera::up;
  }
  {
    auto v = lua.new_usertype<Model>("", sol::no_construction());
    v["aabb"] = &Model::aabb;
    v["load"] = &Model::load;
    v["load_sprite"] = &Model::load_sprite;
    v["load_box"] = &Model::load_box;
    v["bone_uniform_data"] = &Model::bone_uniform_data;
    v["play"] = &Model::play;
    v["update"] = &Model::update;
  }
  {
    auto v = lua.new_usertype<AABB>("", sol::no_construction());
    v["min"] = &AABB::_min;
    v["max"] = &AABB::_max;
    v["update_world"] = &AABB::update_world;
  }
  {
    auto v = lua.new_usertype<Timer>("", sol::no_construction());
    v["start"] = &Timer::start;
    v["stop"] = &Timer::stop;
    v["is_started"] = &Timer::is_started;
    v["set_time"] = &Timer::set_time;
    v["check"] = &Timer::check;
  }
  {
    auto v = lua.new_usertype<UniformData>("", sol::no_construction());
    v["add"] = &UniformData::add;
    v["change"] = &UniformData::change;
  }
  {
    auto v = lua.new_usertype<Shader>("", sol::no_construction());
    v["load_vertex_shader"] = &Shader::load_vertex_shader;
    v["load_fragment_shader"] = &Shader::load_fragment_shader;
    v["compile_and_load_vertex_shader"] =
        &Shader::compile_and_load_vertex_shader;
    v["compile_and_load_fragment_shader"] =
        &Shader::compile_and_load_fragment_shader;
  }
  {
    auto v = lua.new_usertype<RenderPipeline2D>("", sol::no_construction());
    v["set_vertex_shader"] = &RenderPipeline2D::set_vertex_shader;
    v["set_fragment_shader"] = &RenderPipeline2D::set_fragment_shader;
    v["build"] = &RenderPipeline2D::build;
  }
  {
    auto v = lua.new_usertype<RenderPipeline3D>("", sol::no_construction());
    v["set_vertex_shader"] = &RenderPipeline3D::set_vertex_shader;
    v["set_vertex_instanced_shader"] =
        &RenderPipeline3D::set_vertex_instanced_shader;
    v["set_fragment_shader"] = &RenderPipeline3D::set_fragment_shader;
    v["set_animation"] = &RenderPipeline3D::set_animation;
    v["build"] = &RenderPipeline3D::build;
  }
  register_drawable(lua);
  register_table(lua);
  register_graph(lua);

#endif
  return true;
}

} // namespace sinen
