#include "script_backend.hpp"
#include <functional>

#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include <sol/sol.hpp>

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <graphics/graphics.hpp>
#include <math/math.hpp>
#include <physics/physics.hpp>

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

bool LuaScript::Initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                       sol::lib::bit32, sol::lib::io, sol::lib::os,
                       sol::lib::string, sol::lib::debug, sol::lib::table);
  auto lua = state.create_table("sn");
  {
    auto &v = lua;
    state["require"] = [&](const std::string &str) -> sol::object {
      return state.require_script(
          str, AssetIO::OpenAsString(AssetType::Script, str + ".lua"));
    };
    v["Texture"] = []() -> Texture { return Texture(); };
    v["Material"] = []() -> Material { return Material(); };
    v["RenderTexture"] = []() -> RenderTexture { return RenderTexture(); };
    v["Font"] = []() -> Font { return Font(); };
    v["Vec3"] = sol::overload(
        [](const float x, const float y, const float z) -> glm::vec3 {
          return {x, y, z};
        },
        [](const float value) -> glm::vec3 { return glm::vec3(value); });
    v["Vec3i"] = sol::overload(
        [](const int x, const int y, const int z) -> glm::ivec3 {
          return {x, y, z};
        },
        [](const int value) -> glm::ivec3 { return glm::ivec3(value); });
    v["Vec2"] = sol::overload(
        [](const float x, const float y) -> glm::vec2 { return {x, y}; },
        [](const float value) -> glm::vec2 { return glm::vec2{value}; });
    v["Vec2i"] = sol::overload(
        [](const int x, const int y) -> glm::ivec2 { return {x, y}; },
        [](const int value) -> glm::ivec2 { return glm::ivec2{value}; });
    v["Color"] =
        sol::overload([](const float r, const float g, const float b,
                         const float a) -> Color { return {r, g, b, a}; },
                      []() -> Color { return {}; });
    v["Model"] = []() -> Model { return {}; };
    v["Music"] = []() -> Music { return {}; };
    v["Sound"] = []() -> Sound { return {}; };
    v["AABB"] = []() -> AABB { return {}; };
    v["Timer"] = []() -> Timer { return {}; };
    v["Shader"] = []() -> Shader { return {}; };
    v["UniformData"] = []() -> UniformData { return {}; };
    v["GraphicsPipeline2D"] = []() -> GraphicsPipeline2D { return {}; };
    v["GraphicsPipeline3D"] = []() -> GraphicsPipeline3D { return {}; };
    lua["Draw2D"] = sol::overload(
        []() -> Draw2D { return {}; },
        [](const Texture &texture) -> Draw2D { return Draw2D(texture); });
    lua["Draw3D"] = sol::overload(
        []() -> Draw3D { return {}; },
        [](const Texture &texture) -> Draw3D { return Draw3D(texture); });
    lua["Rect"] =
        sol::overload([]() -> Rect { return {}; },
                      [](float x, float y, float width, float height) -> Rect {
                        return Rect(x, y, width, height);
                      },
                      [](const glm::vec2 &p, const glm::vec2 &s) -> Rect {
                        return Rect(p, s);
                      });
    lua["Transform"] = []() -> Transform { return {}; };
    lua["Camera"] = []() -> Camera { return {}; };
    lua["Camera2D"] = []() -> Camera2D { return {}; };
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
    auto v = lua.new_usertype<glm::ivec3>("", sol::no_construction());
    v["x"] = &glm::ivec3::x;
    v["y"] = &glm::ivec3::y;
    v["z"] = &glm::ivec3::z;
    v["__add"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a + b; };
    v["__sub"] = [](const glm::ivec3 &a, const glm::ivec3 &b) { return a - b; };
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
    v["fill"] = &Texture::FillColor;
    v["blend"] = &Texture::BlendColor;
    v["copy"] = &Texture::Copy;
    v["load"] = &Texture::Load;
    v["size"] = &Texture::Size;
  }
  {
    auto v = lua.new_usertype<Material>("", sol::no_construction());
    v["append_texture"] = &Material::AppendTexture;
    v["set_texture"] = sol::overload(
        [](Material &m, const Texture &texture) { m.SetTexture(texture); },
        [](Material &m, const Texture &texture, size_t index) {
          m.SetTexture(texture, index - 1);
        });
    v["get_texture"] = &Material::GetTexture;
    v["clear"] = &Material::Clear;
  }
  {
    auto v = lua.new_usertype<RenderTexture>("", sol::no_construction());
    v["create"] = &RenderTexture::Create;
  }
  {
    auto v = lua.new_usertype<Font>("", sol::no_construction());
    v["load"] = sol::overload(
        [](Font &f, int point_size) { return f.Load(point_size); },
        [](Font &f, int point_size, std::string_view path) {
          return f.Load(point_size, path);
        });
    v["render_text"] = &Font::RenderText;
    v["resize"] = &Font::Resize;
  }
  {
    auto v = lua.new_usertype<Music>("", sol::no_construction());
    v["load"] = &Music::Load;
    v["play"] = &Music::Play;
    v["set_volume"] = &Music::SetVolume;
  }
  {
    auto v = lua.new_usertype<Sound>("", sol::no_construction());
    v["load"] = &Sound::Load;
    v["play"] = &Sound::Play;
    v["set_volume"] = &Sound::SetVolume;
    v["set_pitch"] = &Sound::SetPitch;
    v["set_listener"] = &Sound::SetListener;
    v["set_position"] = &Sound::SetPosition;
  }
  {
    auto v = lua.new_usertype<Camera>("", sol::no_construction());
    v["lookat"] = &Camera::LookAt;
    v["perspective"] = &Camera::Perspective;
    v["orthographic"] = &Camera::Orthographic;
    v["get_position"] = &Camera::GetPosition;
    v["get_target"] = &Camera::GetTarget;
    v["get_up"] = &Camera::GetUp;
    v["is_aabb_in_frustum"] = &Camera::IsAABBInFrustum;
  }
  {
    auto v = lua.new_usertype<Camera2D>("", sol::no_construction());
    v["size"] = &Camera2D::Size;
    v["half"] = &Camera2D::Half;
    v["resize"] = &Camera2D::Resize;
    v["windowRatio"] = &Camera2D::WindowRatio;
    v["inv_window_ratio"] = &Camera2D::InvWindowRatio;
  }
  {
    auto v = lua.new_usertype<Model>("", sol::no_construction());
    v["get_aabb"] = &Model::GetAABB;
    v["load"] = &Model::Load;
    v["load_sprite"] = &Model::LoadSprite;
    v["load_box"] = &Model::LoadBox;
    v["get_bone_uniform_data"] = &Model::GetBoneUniformData;
    v["play"] = &Model::Play;
    v["update"] = &Model::Update;
  }
  {
    auto v = lua.new_usertype<AABB>("", sol::no_construction());
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
    v["update_world"] = &AABB::update_world;
  }
  {
    auto v = lua.new_usertype<Timer>("", sol::no_construction());
    v["start"] = &Timer::Start;
    v["stop"] = &Timer::Stop;
    v["is_started"] = &Timer::IsStarted;
    v["set_time"] = &Timer::SetTime;
    v["check"] = &Timer::Check;
  }
  {
    // Collider
    auto v = lua.new_usertype<Collider>("", sol::no_construction());
    v["get_position"] = &Collider::GetPosition;
    v["get_velocity"] = &Collider::GetVelocity;
    v["set_linear_velocity"] = &Collider::SetLinearVelocity;
  }
  {
    auto v = lua.new_usertype<UniformData>("", sol::no_construction());
    v["add"] = &UniformData::Add;
    v["change"] = &UniformData::Change;
  }
  {
    auto v = lua.new_usertype<Shader>("", sol::no_construction());
    v["load_vertex_shader"] = &Shader::LoadVertexShader;
    v["load_fragment_shader"] = &Shader::LoadFragmentShader;
    v["compile_load_vertex_shader"] = &Shader::CompileAndLoadVertexShader;
    v["compile_load_fragment_shader"] = &Shader::CompileAndLoadFragmentShader;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline2D>("", sol::no_construction());
    v["set_vertex_shader"] = &GraphicsPipeline2D::SetVertexShader;
    v["set_fragment_shader"] = &GraphicsPipeline2D::SetFragmentShader;
    v["build"] = &GraphicsPipeline2D::Build;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline3D>("", sol::no_construction());
    v["set_vertex_shader"] = &GraphicsPipeline3D::SetVertexShader;
    v["set_vertex_instanced_shader"] =
        &GraphicsPipeline3D::SetVertexInstancedShader;
    v["set_fragment_shader"] = &GraphicsPipeline3D::SetFragmentShader;
    v["set_animation"] = &GraphicsPipeline3D::SetAnimation;
    v["build"] = &GraphicsPipeline3D::build;
  }
  {
    auto v = lua.new_usertype<Draw2D>("", sol::no_construction());
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["material"] = &Draw2D::material;
    v["add"] = &Draw2D::Add;
    v["at"] = &Draw2D::At;
    v["clear"] = &Draw2D::Clear;
  }
  {
    auto v = lua.new_usertype<Draw3D>("", sol::no_construction());
    v["position"] = &Draw3D::position;
    v["rotation"] = &Draw3D::rotation;
    v["scale"] = &Draw3D::scale;
    v["material"] = &Draw3D::material;
    v["model"] = &Draw3D::model;
    v["isDrawDepth"] = &Draw3D::isDrawDepth;
    v["add"] = &Draw3D::Add;
    v["at"] = &Draw3D::At;
    v["clear"] = &Draw3D::Clear;
  }
  {
    // Rect
    auto v = lua.new_usertype<Rect>("", sol::no_construction());
    v["x"] = &Rect::x;
    v["y"] = &Rect::y;
    v["width"] = &Rect::width;
    v["height"] = &Rect::height;
  }
  {
    // Transform
    auto v = lua.new_usertype<Transform>("", sol::no_construction());
    v["position"] = &Transform::position;
    v["rotation"] = &Transform::rotation;
    v["scale"] = &Transform::scale;
  }
  {
    auto v = lua.create_named("Random");
    v["get_range"] = &Random::GetRange;
    v["get_int_range"] = &Random::GetIntRange;
  }
  {
    auto v = lua.create_named("Window");
    v["get_name"] = &Window::GetName;
    v["size"] = &Window::Size;
    v["half"] = Window::Half;
    v["resize"] = &Window::Resize;
    v["set_fullscreen"] = &Window::SetFullscreen;
    v["rename"] = &Window::Rename;
    v["resized"] = &Window::Resized;
  }
  {
    auto v = lua.create_named("Physics");
    v["create_box_collider"] = &Physics::CreateBoxCollider;
    v["create_sphere_collider"] = &Physics::CreateSphereCollider;
    v["create_cylinder_collider"] = &Physics::CreateCylinderCollider;
    v["add_collider"] = &Physics::AddCollider;
  }
  {
    auto v = lua.create_named("Graphics");
    v["draw2d"] = &Graphics::Draw2D;
    v["draw3d"] = &Graphics::Draw3D;
    v["draw_rect"] = sol::overload(
        [](const Rect &rect, const Color &color) {
          Graphics::DrawRect(rect, color);
        },
        [](const Rect &rect, const Color &color, float angle) {
          Graphics::DrawRect(rect, color, angle);
        });
    v["draw_image"] = sol::overload(
        [](const Texture &texture, const Rect &rect) {
          Graphics::DrawImage(texture, rect);
        },
        [](const Texture &texture, const Rect &rect, float angle) {
          Graphics::DrawImage(texture, rect, angle);
        });
    v["draw_text"] = sol::overload(
        [](const std::string &text, const glm::vec2 &position) {
          Graphics::DrawText(text, position);
        },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color) { Graphics::DrawText(text, position, color); },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color, float fontSize) {
          Graphics::DrawText(text, position, color, fontSize);
        },
        [](const std::string &text, const glm::vec2 &position,
           const Color &color, float fontSize, float angle) {
          Graphics::DrawText(text, position, color, fontSize, angle);
        });
    v["draw_model"] = &Graphics::DrawModel;
    v["set_camera"] = &Graphics::SetCamera;
    v["get_camera"] = &Graphics::GetCamera;
    v["set_camera2d"] = &Graphics::SetCamera2D;
    v["get_camera2d"] = &Graphics::GetCamera2D;
    v["get_clear_color"] = &Graphics::GetClearColor;
    v["set_clear_color"] = &Graphics::SetClearColor;
    v["bind_pipeline2d"] = &Graphics::BindPipeline2D;
    v["bind_default_pipeline2d"] = &Graphics::BindDefaultPipeline2D;
    v["bind_pipeline3d"] = &Graphics::BindPipeline3D;
    v["bind_default_pipeline3d"] = &Graphics::BindDefaultPipeline3D;
    v["set_uniform_data"] = &Graphics::SetUniformData;
    v["set_render_target"] = &Graphics::SetRenderTarget;
    v["flush"] = &Graphics::Flush;
    v["readback_texture"] = &Graphics::ReadbackTexture;
  }
  {
    auto v = lua.create_named("Collision");
    v["aabb_vs_aabb"] = &Collision::AABBvsAABB;
  }
  {
    auto v = lua.create_named("FileSystem");
    v["enumerate_directory"] = &FileSystem::EnumerateDirectory;
  }
  {
    auto v = lua.create_named("Script");
    v["load"] = sol::overload(
        [&](const std::string &filePath) {
          return Script::Load(filePath, ".");
        },
        [&](const std::string &filePath, const std::string &baseDirPath) {
          return Script::Load(filePath, baseDirPath);
        });
  }
  {
    auto v = lua.create_named("Keyboard");
    v["is_pressed"] = &Keyboard::IsPressed;
    v["is_released"] = &Keyboard::IsReleased;
    v["is_down"] = &Keyboard::IsDown;
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
    v["is_pressed"] = &GamePad::IsPressed;
    v["is_released"] = &GamePad::IsReleased;
    v["is_down"] = &GamePad::IsDown;
    v["get_left_stick"] = &GamePad::GetLeftStick;
    v["get_right_stick"] = &GamePad::GetRightStick;
    v["is_connected"] = &GamePad::IsConnected;
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
    v["seconds"] = &Time::Seconds;
    v["milli"] = &Time::Milli;
    v["deltatime"] = &Time::DeltaTime;
  }
  {
    // logger
    auto v = lua.create_named("Logger");
    v["verbose"] = [](std::string str) { Logger::Verbose("%s", str.data()); };
    v["info"] = [](std::string str) { Logger::Info("%s", str.data()); };
    v["error"] = [](std::string str) { Logger::Error("%s", str.data()); };
    v["warn"] = [](std::string str) { Logger::Warn("%s", str.data()); };
    v["critical"] = [](std::string str) { Logger::Critical("%s", str.data()); };
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
    v["width"] = &Grid<int>::Width;
    v["height"] = &Grid<int>::Height;
    v["size"] = &Grid<int>::Size;
    v["clear"] = &Grid<int>::Clear;
    v["resize"] = &Grid<int>::Resize;
    v["fill"] = [](Grid<int> &g, int value) {
      for (auto &i : g) {
        i = value;
      }
    };
  };
  lua["BFSGrid"] = [](const Grid<int> &g) { return BFSGrid(g); };
  {
    auto v = lua.new_usertype<BFSGrid>("", sol::no_construction());
    v["width"] = &BFSGrid::Width;
    v["height"] = &BFSGrid::Height;
    v["find_path"] = [](BFSGrid &g, const glm::ivec2 &start,
                        const glm::ivec2 &end) {
      return g.FindPath({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.Trace();
      return glm::ivec2{t.x + 1, t.y + 1};
    };
    v["traceable"] = &BFSGrid::Traceable;
    v["reset"] = &BFSGrid::Reset;
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
  state.script(source.data());
}

void LuaScript::Update() { state["update"](); }

void LuaScript::Draw() { state["draw"](); }

} // namespace sinen