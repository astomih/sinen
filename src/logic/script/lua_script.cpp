#include "script.hpp"
#include <functional>
#include <sol/sol.hpp>

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <graphics/graphics.hpp>
#include <logic/logic.hpp>
#include <math/math.hpp>

#include "glm/ext/vector_float3.hpp"
// glm::rotate
#include "glm/ext/vector_int3.hpp"
#include "math/graph/bfs_grid.hpp"
#include "math/graph/grid.hpp"
#include "platform/input/gamepad.hpp"
#include "platform/input/keyboard.hpp"
#include "platform/input/mouse.hpp"
#include "platform/window/window.hpp"
#include "sol/raii.hpp"
#include "sol/types.hpp"
#include <glm/glm.hpp>

#ifdef main
#undef main
#endif

namespace sinen {
class LuaScript final : public IScript {
public:
  LuaScript() = default;
  ~LuaScript() override = default;
  bool Initialize() override;
  void Finalize() override;

  void RunScene(const std::string_view source) override;

  void Update() override;
  void Draw() override;

private:
  sol::state lua;
};

std::unique_ptr<IScript> Script::CreateLua() {
  return std::make_unique<LuaScript>();
}

bool LuaScript::Initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                     sol::lib::bit32, sol::lib::io, sol::lib::os,
                     sol::lib::string, sol::lib::debug, sol::lib::table);
  {
    auto &v = lua;
    v["require"] = [&](const std::string &str) -> sol::object {
      return v.require_script(
          str, DataStream::open_as_string(AssetType::Script, str + ".lua"));
    };
    v["Texture"] = []() -> Texture { return Texture(); };
    v["Material"] = []() -> Material { return Material(); };
    v["RenderTexture"] = []() -> RenderTexture { return RenderTexture(); };
    v["Font"] = []() -> Font { return Font(); };
    v["Vec3"] = [](float x, float y, float z) -> glm::vec3 {
      return glm::vec3(x, y, z);
    };
    v["Vec2"] = [](float x, float y) -> glm::vec2 { return glm::vec2(x, y); };
    v["Point2i"] = [](int x, int y) -> glm::ivec2 { return glm::ivec2(x, y); };
    v["Point2f"] = [](float x, float y) -> glm::vec2 {
      return glm::vec2(x, y);
    };
    v["Quaternion"] = [](sol::this_state s) -> glm::quat {
      return glm::quat();
    };
    v["Color"] = [](float r, float g, float b, float a) -> Color {
      return Color(r, g, b, a);
    };
    v["Model"] = []() -> Model { return Model(); };
    v["Music"] = []() -> Music { return Music(); };
    v["Sound"] = []() -> Sound { return Sound(); };
    v["AABB"] = []() -> AABB { return AABB(); };
    v["Timer"] = []() -> Timer { return Timer(); };
    v["Shader"] = []() -> Shader { return Shader(); };
    v["UniformData"] = []() -> UniformData { return UniformData(); };
    v["GraphicsPipeline2D"] = []() -> GraphicsPipeline2D {
      return GraphicsPipeline2D();
    };
    v["GraphicsPipeline3D"] = []() -> GraphicsPipeline3D {
      return GraphicsPipeline3D();
    };
  }
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
    auto v = lua.new_usertype<Material>("", sol::no_construction());
    v["append"] = &Material::append;
    v["clear"] = &Material::clear;
  }
  {
    auto v = lua.new_usertype<RenderTexture>("", sol::no_construction());
    v["create"] = &RenderTexture::create;
  }
  {
    auto v = lua.new_usertype<Font>("", sol::no_construction());
    v["load"] = &Font::load;
    v["load_from_file"] = &Font::load_from_file;
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
    auto v = lua.new_usertype<GraphicsPipeline2D>("", sol::no_construction());
    v["set_vertex_shader"] = &GraphicsPipeline2D::set_vertex_shader;
    v["set_fragment_shader"] = &GraphicsPipeline2D::set_fragment_shader;
    v["build"] = &GraphicsPipeline2D::build;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline3D>("", sol::no_construction());
    v["set_vertex_shader"] = &GraphicsPipeline3D::set_vertex_shader;
    v["set_vertex_instanced_shader"] =
        &GraphicsPipeline3D::set_vertex_instanced_shader;
    v["set_fragment_shader"] = &GraphicsPipeline3D::set_fragment_shader;
    v["set_animation"] = &GraphicsPipeline3D::set_animation;
    v["build"] = &GraphicsPipeline3D::build;
  }
  lua["Draw2D"] = []() -> Draw2D { return Draw2D(); };
  lua["Draw3D"] = []() -> Draw3D { return Draw3D(); };
  {
    auto v = lua.new_usertype<Draw2D>("", sol::no_construction());
    v["draw"] = &Draw2D::draw;
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["material"] = &Draw2D::material;
    v["user_data_at"] = &Draw2D::user_data_at;
    v["add"] = &Draw2D::add;
    v["at"] = &Draw2D::at;
    v["clear"] = &Draw2D::clear;
  }
  {
    auto v = lua.new_usertype<Draw3D>("", sol::no_construction());
    v["position"] = &Draw3D::position;
    v["rotation"] = &Draw3D::rotation;
    v["scale"] = &Draw3D::scale;
    v["material"] = &Draw3D::material;
    v["draw"] = &Draw3D::draw;
    v["model"] = &Draw3D::model;
    v["is_draw_depth"] = &Draw3D::is_draw_depth;
    v["user_data_at"] = &Draw3D::user_data_at;
    v["add"] = &Draw3D::add;
    v["at"] = &Draw3D::at;
    v["clear"] = &Draw3D::clear;
  }
  {
    auto v = lua.create_table("Random");
    v["get_int_range"] = &Random::get_int_range;
    v["get_float_range"] = &Random::get_float_range;
  }
  {
    auto v = lua.create_table("Window");
    v["name"] = &Window::name;
    v["size"] = &Window::size;
    v["half"] = Window::half;
    v["resize"] = &Window::resize;
    v["set_fullscreen"] = &Window::set_fullscreen;
    v["rename"] = &Window::rename;
    v["resized"] = &Window::resized;
  }
  {
    auto v = lua.create_table("Graphics");
    v["clear_color"] = &Graphics::clear_color;
    v["set_clear_color"] = &Graphics::set_clear_color;
    v["at_render_texture_user_data"] = &Graphics::at_render_texture_user_data;
    v["bind_pipeline2d"] = &Graphics::bind_pipeline2d;
    v["bind_default_pipeline2d"] = &Graphics::bind_default_pipeline2d;
    v["bind_pipeline3d"] = &Graphics::bind_pipeline3d;
    v["bind_default_pipeline3d"] = &Graphics::bind_default_pipeline3d;
    v["set_uniform_data"] = &Graphics::set_uniform_data;
    v["begin_target2d"] = &Graphics::begin_target2d;
    v["begin_target3d"] = &Graphics::begin_target3d;
    v["end_target"] = &Graphics::end_target;
  }
  {
    auto v = lua.create_table("Scene");
    v["camera"] = &Scene::camera;
    v["size"] = &Scene::size;
    v["resize"] = &Scene::resize;
    v["half"] = &Scene::half;
    v["ratio"] = &Scene::ratio;
    v["inv_ratio"] = &Scene::inv_ratio;
    v["delta_time"] = &Scene::delta_time;
    v["change"] = &Scene::change;
  }
  {
    auto v = lua.create_table("Collision");
    v["aabb_aabb"] = &Collision::aabb_aabb;
  }
  {
    auto v = lua.create_table("Keyboard");
    v["is_pressed"] = &Keyboard::is_pressed;
    v["is_released"] = &Keyboard::is_released;
    v["is_down"] = &Keyboard::is_down;
    v["A"] = (int)Keyboard::A;
    v["B"] = (int)Keyboard::B;
    v["C"] = (int)Keyboard::C;
    v["D"] = (int)Keyboard::D;
    v["E"] = (int)Keyboard::E;
    v["F"] = (int)Keyboard::F;
    v["G"] = (int)Keyboard::G;
    v["H"] = (int)Keyboard::H;
    v["I"] = (int)Keyboard::I;
    v["J"] = (int)Keyboard::J;
    v["K"] = (int)Keyboard::K;
    v["L"] = (int)Keyboard::L;
    v["M"] = (int)Keyboard::M;
    v["N"] = (int)Keyboard::N;
    v["O"] = (int)Keyboard::O;
    v["P"] = (int)Keyboard::P;
    v["Q"] = (int)Keyboard::Q;
    v["R"] = (int)Keyboard::R;
    v["S"] = (int)Keyboard::S;
    v["T"] = (int)Keyboard::T;
    v["U"] = (int)Keyboard::U;
    v["V"] = (int)Keyboard::V;
    v["W"] = (int)Keyboard::W;
    v["X"] = (int)Keyboard::X;
    v["Y"] = (int)Keyboard::Y;
    v["Z"] = (int)Keyboard::Z;
    v["key0"] = (int)Keyboard::Key0;
    v["key1"] = (int)Keyboard::Key1;
    v["key2"] = (int)Keyboard::Key2;
    v["key3"] = (int)Keyboard::Key3;
    v["key4"] = (int)Keyboard::Key4;
    v["key5"] = (int)Keyboard::Key5;
    v["key6"] = (int)Keyboard::Key6;
    v["key7"] = (int)Keyboard::Key7;
    v["key8"] = (int)Keyboard::Key8;
    v["key9"] = (int)Keyboard::Key9;
    v["F1"] = (int)Keyboard::F1;
    v["F2"] = (int)Keyboard::F2;
    v["F3"] = (int)Keyboard::F3;
    v["F4"] = (int)Keyboard::F4;
    v["F5"] = (int)Keyboard::F5;
    v["F6"] = (int)Keyboard::F6;
    v["F7"] = (int)Keyboard::F7;
    v["F8"] = (int)Keyboard::F8;
    v["F9"] = (int)Keyboard::F9;
    v["F10"] = (int)Keyboard::F10;
    v["F11"] = (int)Keyboard::F11;
    v["F12"] = (int)Keyboard::F12;
    v["UP"] = (int)Keyboard::UP;
    v["DOWN"] = (int)Keyboard::DOWN;
    v["LEFT"] = (int)Keyboard::LEFT;
    v["RIGHT"] = (int)Keyboard::RIGHT;
    v["ESCAPE"] = (int)Keyboard::ESCAPE;
    v["SPACE"] = (int)Keyboard::SPACE;
    v["ENTER"] = (int)Keyboard::RETURN;
    v["BACKSPACE"] = (int)Keyboard::BACKSPACE;
    v["TAB"] = (int)Keyboard::TAB;
    v["LSHIFT"] = (int)Keyboard::LSHIFT;
    v["RSHIFT"] = (int)Keyboard::RSHIFT;
    v["LCTRL"] = (int)Keyboard::LCTRL;
    v["RCTRL"] = (int)Keyboard::RCTRL;
    v["ALT"] = (int)Keyboard::ALTERASE;
  }
  {
    auto v = lua.create_table("Mouse");
    v["set_relative"] = &Mouse::set_relative;
    v["is_relative"] = &Mouse::is_relative;
    v["is_pressed"] = &Mouse::is_pressed;
    v["is_released"] = &Mouse::is_released;
    v["is_down"] = &Mouse::is_down;
    v["position"] = &Mouse::get_position;
    v["position_on_scene"] = &Mouse::get_position_on_scene;
    v["set_position"] = &Mouse::set_position;
    v["set_position_on_scene"] = &Mouse::set_position_on_scene;
    v["scroll_wheel"] = &Mouse::get_scroll_wheel;
    v["hide_cursor"] = &Mouse::hide_cursor;
    v["LEFT"] = (int)Mouse::LEFT;
    v["RIGHT"] = (int)Mouse::RIGHT;
    v["MIDDLE"] = (int)Mouse::MIDDLE;
    v["X1"] = (int)Mouse::X1;
    v["X2"] = (int)Mouse::X2;
  }
  {
    auto v = lua.create_table("Gamepad");
    v["is_pressed"] = &GamePad::is_pressed;
    v["is_released"] = &GamePad::is_released;
    v["is_down"] = &GamePad::is_down;
    v["left_stick"] = &GamePad::get_left_stick;
    v["right_stick"] = &GamePad::get_right_stick;
    v["is_connected"] = &GamePad::is_connected;
    v["INVALID"] = (int)GamePad::INVALID;
    v["A"] = (int)GamePad::A;
    v["B"] = (int)GamePad::B;
    v["X"] = (int)GamePad::X;
    v["Y"] = (int)GamePad::Y;
    v["BACK"] = (int)GamePad::BACK;
    v["GUIDE"] = (int)GamePad::GUIDE;
    v["START"] = (int)GamePad::START;
    v["LEFTSTICK"] = (int)GamePad::LEFTSTICK;
    v["RIGHTSTICK"] = (int)GamePad::RIGHTSTICK;
    v["LEFTSHOULDER"] = (int)GamePad::LEFTSHOULDER;
    v["RIGHTSHOULDER"] = (int)GamePad::RIGHTSHOULDER;
    v["DPAD_UP"] = (int)GamePad::DPAD_UP;
    v["DPAD_DOWN"] = (int)GamePad::DPAD_DOWN;
    v["DPAD_LEFT"] = (int)GamePad::DPAD_LEFT;
    v["DPAD_RIGHT"] = (int)GamePad::DPAD_RIGHT;
    v["MISC1"] = (int)GamePad::MISC1;
    v["PADDLE1"] = (int)GamePad::PADDLE1;
    v["PADDLE2"] = (int)GamePad::PADDLE2;
    v["PADDLE3"] = (int)GamePad::PADDLE3;
    v["PADDLE4"] = (int)GamePad::PADDLE4;
    v["TOUCHPAD"] = (int)GamePad::TOUCHPAD;
  }
  {
    auto v = lua.create_table("Periodic");
    v["sin0_1"] = [](float period, float t) {
      return Periodic::sin0_1(period, t);
    };
    v["cos0_1"] = [](float period, float t) {
      return Periodic::cos0_1(period, t);
    };
  }
  {
    auto v = lua.create_table("Time");
    v["seconds"] = &Time::seconds;
    v["milli"] = &Time::milli;
  }
  {
    // logger
    auto v = lua.create_table("Logger");
    v["verbose"] = [](std::string str) { Logger::verbose("%s", str.data()); };
    v["debug"] = [](std::string str) { Logger::debug("%s", str.data()); };
    v["info"] = [](std::string str) { Logger::info("%s", str.data()); };
    v["error"] = [](std::string str) { Logger::error("%s", str.data()); };
    v["warn"] = [](std::string str) { Logger::warn("%s", str.data()); };
    v["critical"] = [](std::string str) { Logger::critical("%s", str.data()); };
  }
  lua["Grid"] = [](int width, int height) -> Grid<int> {
    return Grid<int>(width, height);
  };
  {
    auto v = lua.new_usertype<Grid<int>>("", sol::no_construction());
    v["at"] = [](Grid<int> &g, int x, int y) { return g.at(x - 1, y - 1); };
    v["set"] = [](Grid<int> &g, int x, int y, int v) {
      return g.at(x - 1, y - 1) = v;
    };
    v["width"] = &Grid<int>::width;
    v["height"] = &Grid<int>::height;
    v["size"] = &Grid<int>::size;
    v["clear"] = &Grid<int>::clear;
    v["resize"] = &Grid<int>::resize;
    v["fill"] = [](Grid<int> &g, int value) {
      for (auto &i : g) {
        i = value;
      }
    };
  };
  lua["BFSGrid"] = [](const Grid<int> &g) { return BFSGrid(g); };
  {
    auto v = lua.new_usertype<BFSGrid>("", sol::no_construction());
    v["width"] = &BFSGrid::width;
    v["height"] = &BFSGrid::height;
    v["find_path"] = [](BFSGrid &g, const glm::ivec2 &start,
                        const glm::ivec2 &end) {
      return g.find_path({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.trace();
      return glm::ivec2{t.x + 1, t.y + 1};
    };
    v["traceable"] = &BFSGrid::traceable;
    v["reset"] = &BFSGrid::reset;
  }
#endif
  return true;
}

void LuaScript::Finalize() {
#ifndef SINEN_NO_USE_SCRIPT
  lua.collect_garbage();
#endif // SINEN_NO_USE_SCRIPT
}

void LuaScript::RunScene(const std::string_view source) {
  lua.script(source.data());
}

void LuaScript::Update() { lua["update"](); }

void LuaScript::Draw() { lua["draw"](); }

} // namespace sinen