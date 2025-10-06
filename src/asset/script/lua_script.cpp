#include "script_backend.hpp"
#include <functional>

#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include <sol/sol.hpp>

#include "../../main_system.hpp"
#include "glm/ext/vector_float3.hpp"
#include <asset/asset.hpp>
#include <core/core.hpp>
#include <core/io/file_system.hpp>
#include <graphics/graphics.hpp>
#include <math/math.hpp>
#include <physics/physics.hpp>
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

namespace sinen {
class LuaScript final : public IScriptBackend {
public:
  LuaScript() = default;
  ~LuaScript() override = default;
  bool Initialize() override;
  void Finalize() override;

  void RunScene(const std::string_view source) override;

  void Update() override;
  void Draw() override;

private:
  sol::state state;
};

std::unique_ptr<IScriptBackend> ScriptBackend::CreateLua() {
  return std::make_unique<LuaScript>();
}
template <typename T>
static sol::usertype<T> registerClass(sol::table &namespaceTable,
                                      const std::string &name) {
  namespaceTable[name] = []() -> T { return T(); };
  return namespaceTable.new_usertype<T>("", sol::no_construction());
}

template <typename T, typename... Args>
static sol::usertype<T> registerClass(sol::table &namespaceTable,
                                      const std::string &name, Args &&...args) {
  namespaceTable[name] =
      sol::overload_set<std::decay_t<Args>...>(std::forward<Args>(args)...);
  return namespaceTable.new_usertype<T>("", sol::no_construction());
}
bool LuaScript::Initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                       sol::lib::bit32, sol::lib::io, sol::lib::os,
                       sol::lib::string, sol::lib::debug, sol::lib::table);
  auto lua = state.create_table("sn");
  {
    auto &v = lua;
    state["__original_require"] = [&](const std::string &path,
                                      const std::string &base) -> sol::object {
      std::string cwd = base;
      if (path.find_last_of('/') != std::string::npos) {
        cwd += "/" + path.substr(0, path.find_last_of('/'));
      }
      std::string header = "local __CWD__ = '" + cwd + "';" +
                           "local function require(module)"
                           "    __CWD_global__ = __CWD__"
                           "    return __original_require(module, __CWD__);"
                           "end;";
      return state.require_script(
          path, header + AssetIO::openAsString(AssetType::Script,
                                               base + "/" + path + ".lua"));
    };
  }
  {
    auto v = registerClass<glm::vec3>(
        lua, "Vec3",
        [](const float x, const float y, const float z) -> glm::vec3 {
          return {x, y, z};
        },
        [](const float value) -> glm::vec3 { return glm::vec3(value); });
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
    auto v = registerClass<glm::ivec3>(
        lua, "Vec3i",
        [](const int x, const int y, const int z) -> glm::ivec3 {
          return {x, y, z};
        },
        [](const int value) -> glm::ivec3 { return glm::ivec3(value); });
    v["x"] = &glm::ivec3::x;
    v["y"] = &glm::ivec3::y;
    v["z"] = &glm::ivec3::z;
    v["__add"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a - b; };
  }
  {
    auto v = registerClass<glm::vec2>(
        lua, "Vec2",
        [](const float x, const float y) -> glm::vec2 { return {x, y}; },
        [](const float value) -> glm::vec2 { return glm::vec2(value); });
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
    auto v = registerClass<glm::ivec2>(
        lua, "Vec2i",
        [](const int x, const int y) -> glm::ivec2 { return {x, y}; },
        [](const int value) -> glm::ivec2 { return glm::ivec2(value); });
    v["x"] = &glm::ivec2::x;
    v["y"] = &glm::ivec2::y;
    v["__add"] = [](const glm::ivec2 &a, const glm::ivec2 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec2 &a, const glm::ivec2 &b) { return a - b; };
  }
  {
    auto v = registerClass<glm::ivec3>(
        lua, "Vec3i",
        [](const int x, const int y, const int z) -> glm::ivec3 {
          return {x, y, z};
        },
        [](const int value) -> glm::ivec3 { return glm::ivec3(value); });
    v["x"] = &glm::ivec3::x;
    v["y"] = &glm::ivec3::y;
    v["z"] = &glm::ivec3::z;
    v["__add"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a - b; };
  }
  {
    auto v = registerClass<Color>(
        lua, "Color",
        [](const float r, const float g, const float b,
           const float a) -> Color { return {r, g, b, a}; },
        [](float value) -> Color { return {value}; },
        [](float value, float alpha) -> Color { return {value, alpha}; },
        []() -> Color { return {}; });
    v["r"] = &Color::r;
    v["g"] = &Color::g;
    v["b"] = &Color::b;
    v["a"] = &Color::a;
  }
  {
    auto v = registerClass<Texture>(lua, "Texture");
    v["fill"] = &Texture::fill;
    v["blend"] = &Texture::blend;
    v["copy"] = &Texture::copy;
    v["load"] = &Texture::load;
    v["size"] = &Texture::size;
  }
  {
    auto v = registerClass<Material>(lua, "Material");
    v["append_texture"] = &Material::appendTexture;
    v["set_texture"] = sol::overload(
        [](Material &m, const Texture &texture) { m.setTexture(texture); },
        [](Material &m, const Texture &texture, size_t index) {
          m.setTexture(texture, index - 1);
        });
    v["get_texture"] = &Material::getTexture;
    v["clear"] = &Material::clear;
  }
  {
    auto v = registerClass<RenderTexture>(lua, "RenderTexture");
    v["create"] = &RenderTexture::create;
  }
  {
    auto v = registerClass<Font>(lua, "Font");
    v["load"] = sol::overload(
        [](Font &f, int point_size) { return f.load(point_size); },
        [](Font &f, int point_size, std::string_view path) {
          return f.load(point_size, path);
        });
    v["render_text"] = &Font::renderText;
    v["resize"] = &Font::resize;
  }
  {
    auto v = registerClass<Music>(lua, "Music");
    v["load"] = &Music::load;
    v["play"] = &Music::play;
    v["set_volume"] = &Music::setVolume;
  }
  {
    auto v = registerClass<Sound>(lua, "Sound");
    v["load"] = &Sound::load;
    v["play"] = &Sound::play;
    v["set_volume"] = &Sound::setVolume;
    v["set_pitch"] = &Sound::setPitch;
    v["set_listener"] = &Sound::setListener;
    v["set_position"] = &Sound::setPosition;
  }
  {
    auto v = registerClass<Camera>(lua, "Camera");
    v["lookat"] = &Camera::lookat;
    v["perspective"] = &Camera::perspective;
    v["orthographic"] = &Camera::orthographic;
    v["get_position"] = &Camera::getPosition;
    v["get_target"] = &Camera::getTarget;
    v["get_up"] = &Camera::getUp;
    v["is_aabb_in_frustum"] = &Camera::isAABBInFrustum;
  }
  {
    auto v = registerClass<Camera2D>(lua, "Camera2D");
    v["size"] = &Camera2D::size;
    v["half"] = &Camera2D::half;
    v["resize"] = &Camera2D::resize;
    v["window_ratio"] = &Camera2D::windowRatio;
    v["inv_window_ratio"] = &Camera2D::invWindowRatio;
  }
  {
    auto v = registerClass<Model>(lua, "Model");
    v["get_aabb"] = &Model::getAABB;
    v["load"] = &Model::load;
    v["load_sprite"] = &Model::loadSprite;
    v["load_box"] = &Model::loadBox;
    v["get_bone_uniform_data"] = &Model::getBoneUniformData;
    v["play"] = &Model::play;
    v["update"] = &Model::update;
  }
  {
    auto v = registerClass<AABB>(lua, "AABB");
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
    v["update_world"] = &AABB::updateWorld;
  }
  {
    auto v = registerClass<Timer>(lua, "Timer");
    v["start"] = &Timer::start;
    v["stop"] = &Timer::stop;
    v["is_started"] = &Timer::isStarted;
    v["set_time"] = &Timer::setTime;
    v["check"] = &Timer::check;
  }
  {
    // Collider
    auto v = registerClass<Collider>(lua, "Collider");
    v["get_position"] = &Collider::getPosition;
    v["get_velocity"] = &Collider::getVelocity;
    v["set_linear_velocity"] = &Collider::setLinearVelocity;
  }
  {
    auto v = registerClass<UniformData>(lua, "UniformData");
    v["add"] = &UniformData::add;
    v["change"] = &UniformData::change;
  }
  {

    auto v = registerClass<Shader>(lua, "Shader");
    v["load_vertex_shader"] = &Shader::loadVertexShader;
    v["load_fragment_shader"] = &Shader::loadFragmentShader;
    v["compile_load_vertex_shader"] = &Shader::compileAndLoadVertexShader;
    v["compile_load_fragment_shader"] = &Shader::compileAndLoadFragmentShader;
  }
  {
    auto v = registerClass<GraphicsPipeline2D>(lua, "GraphicsPipeline2D");
    v["set_vertex_shader"] = &GraphicsPipeline2D::setVertexShader;
    v["set_fragment_shader"] = &GraphicsPipeline2D::setFragmentShader;
    v["build"] = &GraphicsPipeline2D::build;
  }
  {
    auto v = registerClass<GraphicsPipeline3D>(lua, "GraphicsPipeline3D");
    v["set_vertex_shader"] = &GraphicsPipeline3D::setVertexShader;
    v["set_vertex_instanced_shader"] =
        &GraphicsPipeline3D::setVertexInstancedShader;
    v["set_fragment_shader"] = &GraphicsPipeline3D::setFragmentShader;
    v["set_animation"] = &GraphicsPipeline3D::setAnimation;
    v["build"] = &GraphicsPipeline3D::build;
  }
  {
    auto v = registerClass<Draw2D>(
        lua, "Draw2D",
        [](const Texture &texture) -> Draw2D { return Draw2D(texture); },
        []() -> Draw2D { return {}; });
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["material"] = &Draw2D::material;
    v["add"] = &Draw2D::add;
    v["at"] = &Draw2D::at;
    v["clear"] = &Draw2D::clear;
  }
  {
    auto v = registerClass<Draw3D>(
        lua, "Draw3D",
        [](const Texture &texture) -> Draw3D { return Draw3D(texture); },
        []() -> Draw3D { return {}; });
    v["position"] = &Draw3D::position;
    v["rotation"] = &Draw3D::rotation;
    v["scale"] = &Draw3D::scale;
    v["material"] = &Draw3D::material;
    v["model"] = &Draw3D::model;
    v["add"] = &Draw3D::add;
    v["at"] = &Draw3D::at;
    v["clear"] = &Draw3D::clear;
  }
  {
    // Rect
    auto v = registerClass<Rect>(
        lua, "Rect",
        [](float x, float y, float width, float height) -> Rect {
          return Rect(x, y, width, height);
        },
        [](const glm::vec2 &p, const glm::vec2 &s) -> Rect {
          return Rect(p, s);
        },
        []() -> Rect { return {}; });
    v["x"] = &Rect::x;
    v["y"] = &Rect::y;
    v["width"] = &Rect::width;
    v["height"] = &Rect::height;
  }
  {
    // Transform
    auto v = registerClass<Transform>(lua, "Transform");
    v["position"] = &Transform::position;
    v["rotation"] = &Transform::rotation;
    v["scale"] = &Transform::scale;
  }
  {
    auto v = registerClass<Grid<int>>(lua, "Grid", [](int width, int height) {
      return Grid<int>(width, height);
    });
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
  }
  {
    auto v = registerClass<BFSGrid>(
        lua, "BFSGrid", [](const Grid<int> &g) { return BFSGrid(g); });
    v["width"] = &BFSGrid::width;
    v["height"] = &BFSGrid::height;
    v["find_path"] = [](BFSGrid &g, const glm::ivec2 &start,
                        const glm::ivec2 &end) {
      return g.findPath({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.trace();
      return glm::ivec2{t.x + 1, t.y + 1};
    };
    v["traceable"] = &BFSGrid::traceable;
    v["reset"] = &BFSGrid::reset;
  }
  {
    auto v = lua.create_named("Random");
    v["get_range"] = &Random::getRange;
    v["get_int_range"] = &Random::getIntRange;
  }
  {
    auto v = lua.create_named("Window");
    v["get_name"] = &Window::getName;
    v["size"] = &Window::size;
    v["half"] = Window::half;
    v["resize"] = &Window::resize;
    v["set_fullscreen"] = &Window::setFullscreen;
    v["rename"] = &Window::rename;
    v["resized"] = &Window::resized;
  }
  {
    auto v = lua.create_named("Physics");
    v["create_box_collider"] = &Physics::createBoxCollider;
    v["create_sphere_collider"] = &Physics::createSphereCollider;
    v["create_cylinder_collider"] = &Physics::createCylinderCollider;
    v["add_collider"] = &Physics::addCollider;
  }
  {
    auto v = lua.create_named("Graphics");
    v["draw2d"] = &Graphics::draw2D;
    v["draw3d"] = &Graphics::draw3D;
    v["draw_rect"] = sol::overload(
        [](const Rect &rect, const Color &color) {
          Graphics::drawRect(rect, color);
        },
        [](const Rect &rect, const Color &color, float angle) {
          Graphics::drawRect(rect, color, angle);
        });
    v["draw_image"] = sol::overload(
        [](const Texture &texture, const Rect &rect) {
          Graphics::drawImage(texture, rect);
        },
        [](const Texture &texture, const Rect &rect, float angle) {
          Graphics::drawImage(texture, rect, angle);
        });
    v["draw_text"] = sol::overload(
        [](const std::string &text, const glm::vec2 &position) {
          Graphics::drawText(text, position);
        },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color) { Graphics::drawText(text, position, color); },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color, float fontSize) {
          Graphics::drawText(text, position, color, fontSize);
        },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color, float fontSize, float angle) {
          Graphics::drawText(text, position, color, fontSize, angle);
        });
    v["draw_model"] = &Graphics::drawModel;
    v["set_camera"] = &Graphics::setCamera;
    v["get_camera"] = &Graphics::getCamera;
    v["set_camera2d"] = &Graphics::setCamera2D;
    v["get_camera2d"] = &Graphics::getCamera2D;
    v["get_clear_color"] = &Graphics::getClearColor;
    v["set_clear_color"] = &Graphics::setClearColor;
    v["bind_pipeline2d"] = &Graphics::bindPipeline2D;
    v["bind_default_pipeline2d"] = &Graphics::bindDefaultPipeline2D;
    v["bind_pipeline3d"] = &Graphics::bindPipeline3D;
    v["bind_default_pipeline3d"] = &Graphics::bindDefaultPipeline3D;
    v["set_uniform_data"] = &Graphics::setUniformData;
    v["set_render_target"] = &Graphics::setRenderTarget;
    v["flush"] = &Graphics::flush;
    v["readback_texture"] = &Graphics::readbackTexture;
  }
  {
    auto v = lua.create_named("Collision");
    v["aabb_vs_aabb"] = &Collision::AABBvsAABB;
  }
  {
    auto v = lua.create_named("FileSystem");
    v["enumerate_directory"] = &FileSystem::enumerateDirectory;
  }
  {
    auto v = lua.create_named("Script");
    v["load"] = sol::overload(
        [&](const std::string &filePath) {
          return Script::load(filePath, ".");
        },
        [&](const std::string &filePath, const std::string &baseDirPath) {
          return Script::load(filePath, baseDirPath);
        });
  }
  {
    auto v = lua.create_named("Keyboard");
    v["is_pressed"] = &Keyboard::isPressed;
    v["is_released"] = &Keyboard::isReleased;
    v["is_down"] = &Keyboard::isDown;
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
    auto v = lua.create_named("Mouse");
    v["set_relative"] = &Mouse::SetRelative;
    v["is_relative"] = &Mouse::IsRelative;
    v["is_pressed"] = &Mouse::IsPressed;
    v["is_released"] = &Mouse::IsReleased;
    v["is_down"] = &Mouse::IsDown;
    v["get_position"] = &Mouse::GetPosition;
    v["get_position_on_scene"] = &Mouse::GetPositionOnScene;
    v["set_position"] = &Mouse::SetPosition;
    v["set_position_on_scene"] = &Mouse::SetPositionOnScene;
    v["get_scroll_wheel"] = &Mouse::GetScrollWheel;
    v["hide_cursor"] = &Mouse::HideCursor;
    v["LEFT"] = (int)Mouse::LEFT;
    v["RIGHT"] = (int)Mouse::RIGHT;
    v["MIDDLE"] = (int)Mouse::MIDDLE;
    v["X1"] = (int)Mouse::X1;
    v["X2"] = (int)Mouse::X2;
  }
  {
    auto v = lua.create_named("Gamepad");
    v["is_pressed"] = &GamePad::isPressed;
    v["is_released"] = &GamePad::isReleased;
    v["is_down"] = &GamePad::isDown;
    v["get_left_stick"] = &GamePad::getLeftStick;
    v["get_right_stick"] = &GamePad::getRightStick;
    v["is_connected"] = &GamePad::isConnected;
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
    auto v = lua.create_named("Periodic");
    v["sin0_1"] = [](float period, float t) {
      return Periodic::sin0_1(period, t);
    };
    v["Cos0_1"] = [](float period, float t) {
      return Periodic::cos0_1(period, t);
    };
  }
  {
    auto v = lua.create_named("Time");
    v["seconds"] = &Time::seconds;
    v["milli"] = &Time::milli;
    v["delta"] = &Time::deltaTime;
  }
  {
    // logger
    auto v = lua.create_named("Logger");
    v["verbose"] = [](std::string str) { Logger::verbose("%s", str.data()); };
    v["info"] = [](std::string str) { Logger::info("%s", str.data()); };
    v["error"] = [](std::string str) { Logger::error("%s", str.data()); };
    v["warn"] = [](std::string str) { Logger::warn("%s", str.data()); };
    v["critical"] = [](std::string str) { Logger::critical("%s", str.data()); };
  }
#endif
  return true;
}

void LuaScript::Finalize() {
#ifndef SINEN_NO_USE_SCRIPT
  state.collect_garbage();
#endif // SINEN_NO_USE_SCRIPT
}

void LuaScript::RunScene(const std::string_view source) {
  std::string cwd = ".";
  std::string header = "local __CWD__ = '" + cwd + "' ;" +
                       "local function require(module) ;"
                       "    return __original_require(module, __CWD__) ;"
                       "end ;";

  std::string final = header + std::string(source);

  state.script(final);
}

void LuaScript::Update() { state["update"](); }

void LuaScript::Draw() { state["draw"](); }

} // namespace sinen