#include "../include/script_engine.hpp"
#include <functional>
#include <sol/sol.hpp>

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <logic/logic.hpp>
#include <math/math.hpp>
#include <render/render.hpp>

#include "glm/ext/vector_float3.hpp"
// glm::rotate
#include "glm/ext/vector_int3.hpp"
#include "pybind11/internal/module.h"
#include "register_script.hpp"
#include "render/drawable/drawable_wrapper.hpp"
#include "render/render_pipeline.hpp"
#include "sol/raii.hpp"
#include "sol/types.hpp"
#include <glm/glm.hpp>

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
    auto v = lua.new_usertype<glm::vec3>("", sol::no_construction());
    v["x"] = &glm::vec3::x;
    v["y"] = &glm::vec3::y;
    v["z"] = &glm::vec3::z;
    v["__add"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a + b; };
    v["__sub"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a - b; };
    v["__mul"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a * b; };
    v["__div"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a / b; };
    v["copy"] = [](const glm::vec3 &a) { return a; };
    v["length"] = [](const glm::vec3 &a) { return glm::length(a); };
    v["forward"] = [](const glm::vec3 v,
                      const glm::vec3 rotation) -> glm::vec3 {
      // rotation to mat
      glm::mat4 m =
          glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
      m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
      m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
      // forward
      glm::vec4 forward = m * glm::vec4(v.x, v.y, v.z, 1.0f);
      forward.x = forward.x / forward.w;
      forward.y = forward.y / forward.w;
      forward.z = forward.z / forward.w;

      return glm::vec3(forward.x, forward.y, forward.z);
    };
    v["normalize"] = [](const glm::vec3 &v) { return glm::normalize(v); };
    v["dot"] = [](const glm::vec3 &a, const glm::vec3 &b) {
      return glm::dot(a, b);
    };
    v["cross"] = [](const glm::vec3 &a, const glm::vec3 &b) {
      return glm::cross(a, b);
    };
    v["lerp"] = [](const glm::vec3 &a, const glm::vec3 &b, float t) {
      return glm::mix(a, b, t);
    };
    v["reflect"] = [](const glm::vec3 &v, const glm::vec3 &n) {
      return glm::reflect(v, n);
    };
  }
  {
    auto v = lua.new_usertype<glm::vec2>("", sol::no_construction());
    v["x"] = &glm::vec2::x;
    v["y"] = &glm::vec2::y;
    v["__add"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a + b; };
    v["__sub"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a - b; };
    v["__mul"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a * b; };
    v["__div"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a / b; };
    v["copy"] = [](const glm::vec2 &a) { return a; };
    v["length"] = [](const glm::vec2 &a) { return glm::length(a); };
    v["normalize"] = [](const glm::vec2 &v) { return glm::normalize(v); };
    v["dot"] = [](const glm::vec2 &a, const glm::vec2 &b) {
      return glm::dot(a, b);
    };
    v["lerp"] = [](const glm::vec2 &a, const glm::vec2 &b, float t) {
      return glm::mix(a, b, t);
    };
    v["reflect"] = [](const glm::vec2 &v, const glm::vec2 &n) {
      return glm::reflect(v, n);
    };
  }
  {
    auto v = lua.new_usertype<glm::ivec2>("", sol::no_construction());
    v["x"] = &glm::ivec2::x;
    v["y"] = &glm::ivec2::y;
    v["__add"] = [](const glm::ivec2 &a, const glm::ivec2 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec2 &a, const glm::ivec2 &b) { return a - b; };
  }
  {
    auto v = lua.new_usertype<glm::ivec3>("", sol::no_construction());
    v["x"] = &glm::ivec3::x;
    v["y"] = &glm::ivec3::y;
    v["z"] = &glm::ivec3::z;
    v["__add"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a - b; };
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
    v["is_aabb_in_frustum"] = &Camera::is_aabb_in_frustum;
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
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
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

PYBIND11_EMBEDDED_MODULE(sinen, m) {
  py::class_<glm::vec3>(m, "Vec3")
      .def(py::init<>())
      .def(py::init<float>())
      .def(py::init<float, float, float>())
      .def_readwrite("x", &glm::vec3::x)
      .def_readwrite("y", &glm::vec3::y)
      .def_readwrite("z", &glm::vec3::z)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self * py::self)
      .def(py::self / py::self)
      .def("copy", [](glm::vec3 &a) { return a; })
      .def("length", [](const glm::vec3 &v) { return glm::length(v); })
      .def("forward",
           [](const glm::vec3 v, const glm::vec3 rotation) -> glm::vec3 {
             // rotation to mat
             glm::mat4 m =
                 glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
             m = glm::rotate(m, rotation.y, glm::vec3(0, 1, 0));
             m = glm::rotate(m, rotation.z, glm::vec3(0, 0, 1));
             // forward
             glm::vec4 forward = m * glm::vec4(v.x, v.y, v.z, 1.0f);
             forward.x = forward.x / forward.w;
             forward.y = forward.y / forward.w;
             forward.z = forward.z / forward.w;

             return glm::vec3(forward.x, forward.y, forward.z);
           })
      .def("normalize", [](const glm::vec3 &v) { return glm::normalize(v); })
      .def("dot", [](const glm::vec3 &a,
                     const glm::vec3 &b) { return glm::dot(a, b); })
      .def("cross", [](const glm::vec3 &a,
                       const glm::vec3 &b) { return glm::cross(a, b); })
      .def("lerp", [](const glm::vec3 &a, const glm::vec3 &b,
                      float t) { return glm::mix(a, b, t); })
      .def("reflect", [](const glm::vec3 &v, const glm::vec3 &n) {
        return glm::reflect(v, n);
      });

  // vec2
  py::class_<glm::vec2>(m, "Vec2")
      .def(py::init<>())
      .def(py::init<float>())
      .def(py::init<float, float>())
      .def_readwrite("x", &glm::vec2::x)
      .def_readwrite("y", &glm::vec2::y)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self * py::self)
      .def(py::self / py::self)
      .def("copy", [](glm::vec2 &a) { return a; })
      .def("length", [](const glm::vec2 &a) { return glm::length(a); })
      .def("normalize", [](const glm::vec2 &v) { return glm::normalize(v); })
      .def("dot", [](const glm::vec2 &a,
                     const glm::vec2 &b) { return glm::dot(a, b); })
      .def("lerp", [](const glm::vec2 &a, const glm::vec2 &b,
                      float t) { return glm::mix(a, b, t); })
      .def("reflect", [](const glm::vec2 &v, const glm::vec2 &n) {
        return glm::reflect(v, n);
      });

  // Texture
  py::class_<Texture>(m, "Texture")
      .def(py::init<>())
      .def("fill_color", &Texture::fill_color)
      .def("blend_color", &Texture::blend_color)
      .def("copy", &Texture::copy)
      .def("load", &Texture::load)
      .def("size", &Texture::size);
  // Draw2D
  py::class_<Draw2D>(m, "Draw2D")
      .def(py::init<>())
      .def(py::init<Texture>())
      .def("draw", &Draw2D::draw)
      .def_readwrite("scale", &Draw2D::scale)
      .def_readwrite("position", &Draw2D::position)
      .def_readwrite("rotation", &Draw2D::rotation);
  // Font
  py::class_<Font>(m, "Font")
      .def(py::init<>())
      .def("load", &Font::load)
      .def("render_text", &Font::render_text)
      .def("resize", &Font::resize);
  // Color
  py::class_<Color>(m, "Color")
      .def(py::init<>())
      .def(py::init<float, float, float, float>())
      .def_readwrite("r", &Color::r)
      .def_readwrite("g", &Color::g)
      .def_readwrite("b", &Color::b)
      .def_readwrite("a", &Color::a);
}
} // namespace sinen
