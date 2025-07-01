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
  sol::state state;
};

std::unique_ptr<IScript> Script::CreateLua() {
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
    v["require"] = [&](const std::string &str) -> sol::object {
      return state.require_script(
          str, DataStream::OpenAsString(AssetType::Script, str + ".lua"));
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
                      []() -> Color { return Color{}; });
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
    lua["Draw2D"] = sol::overload(
        []() -> Draw2D { return Draw2D(); },
        [](const Texture &texture) -> Draw2D { return Draw2D(texture); });
    lua["Draw3D"] = sol::overload(
        []() -> Draw3D { return Draw3D(); },
        [](const Texture &texture) -> Draw3D { return Draw3D(texture); });
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
    v["Copy"] = [](const glm::vec3 &a) { return a; };
    v["Length"] = [](const glm::vec3 &a) { return glm::length(a); };
    v["Forward"] = [](const glm::vec3 v,
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
    v["Normalize"] = [](const glm::vec3 &v) { return glm::normalize(v); };
    v["Dot"] = [](const glm::vec3 &a, const glm::vec3 &b) {
      return glm::dot(a, b);
    };
    v["Cross"] = [](const glm::vec3 &a, const glm::vec3 &b) {
      return glm::cross(a, b);
    };
    v["Lerp"] = [](const glm::vec3 &a, const glm::vec3 &b, float t) {
      return glm::mix(a, b, t);
    };
    v["Reflect"] = [](const glm::vec3 &v, const glm::vec3 &n) {
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
    v["Copy"] = [](const glm::vec2 &a) { return a; };
    v["Length"] = [](const glm::vec2 &a) { return glm::length(a); };
    v["Normalize"] = [](const glm::vec2 &v) { return glm::normalize(v); };
    v["Dot"] = [](const glm::vec2 &a, const glm::vec2 &b) {
      return glm::dot(a, b);
    };
    v["Lerp"] = [](const glm::vec2 &a, const glm::vec2 &b, float t) {
      return glm::mix(a, b, t);
    };
    v["Reflect"] = [](const glm::vec2 &v, const glm::vec2 &n) {
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
    v["FillColor"] = &Texture::FillColor;
    v["BlendColor"] = &Texture::BlendColor;
    v["Copy"] = &Texture::Copy;
    v["Load"] = &Texture::Load;
    v["Size"] = &Texture::Size;
  }
  {
    auto v = lua.new_usertype<Material>("", sol::no_construction());
    v["AppendTexture"] = &Material::AppendTexture;
    v["SetTexture"] = sol::overload(
        [](Material &m, const Texture &texture) { m.SetTexture(texture); },
        [](Material &m, const Texture &texture, size_t index) {
          m.SetTexture(texture, index - 1);
        });
    v["GetTexture"] = &Material::GetTexture;
    v["Clear"] = &Material::Clear;
  }
  {
    auto v = lua.new_usertype<RenderTexture>("", sol::no_construction());
    v["Create"] = &RenderTexture::Create;
  }
  {
    auto v = lua.new_usertype<Font>("", sol::no_construction());
    v["Load"] = sol::overload(
        [](Font &f, int point_size) { return f.Load(point_size); },
        [](Font &f, int point_size, std::string_view path) {
          return f.Load(point_size, path);
        });
    v["RenderText"] = &Font::RenderText;
    v["Resize"] = &Font::Resize;
  }
  {
    auto v = lua.new_usertype<Music>("", sol::no_construction());
    v["Load"] = &Music::Load;
    v["Play"] = &Music::Play;
    v["SetVolume"] = &Music::SetVolume;
  }
  {
    auto v = lua.new_usertype<Sound>("", sol::no_construction());
    v["Load"] = &Sound::Load;
    v["Play"] = &Sound::Play;
    v["SetVolume"] = &Sound::SetVolume;
    v["SetPitch"] = &Sound::SetPitch;
    v["SetListener"] = &Sound::SetListener;
    v["SetPosition"] = &Sound::SetPosition;
  }
  {
    auto v = lua.new_usertype<Camera>("", sol::no_construction());
    v["LookAt"] = &Camera::LookAt;
    v["Perspective"] = &Camera::Perspective;
    v["Orthographic"] = &Camera::Orthographic;
    v["GetPosition"] = &Camera::GetPosition;
    v["GetTarget"] = &Camera::GetTarget;
    v["GetUp"] = &Camera::GetUp;
    v["IsAABBInFrustum"] = &Camera::IsAABBInFrustum;
  }
  {
    auto v = lua.new_usertype<Model>("", sol::no_construction());
    v["GetAABB"] = &Model::GetAABB;
    v["Load"] = &Model::Load;
    v["LoadSprite"] = &Model::LoadSprite;
    v["LoadBox"] = &Model::LoadBox;
    v["GetBoneUniformData"] = &Model::GetBoneUniformData;
    v["Play"] = &Model::Play;
    v["Update"] = &Model::Update;
  }
  {
    auto v = lua.new_usertype<AABB>("", sol::no_construction());
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
    v["UpdateWorld"] = &AABB::update_world;
  }
  {
    auto v = lua.new_usertype<Timer>("", sol::no_construction());
    v["Start"] = &Timer::Start;
    v["Stop"] = &Timer::Stop;
    v["IsStarted"] = &Timer::IsStarted;
    v["SetTime"] = &Timer::SetTime;
    v["Check"] = &Timer::Check;
  }
  {
    auto v = lua.new_usertype<UniformData>("", sol::no_construction());
    v["Add"] = &UniformData::Add;
    v["Change"] = &UniformData::Change;
  }
  {
    auto v = lua.new_usertype<Shader>("", sol::no_construction());
    v["LoadVertexShader"] = &Shader::LoadVertexShader;
    v["LoadFragmentShader"] = &Shader::LoadFragmentShader;
    v["CompileAndLoadVertexShader"] = &Shader::CompileAndLoadVertexShader;
    v["CompileAndLoadFragmentShader"] = &Shader::CompileAndLoadFragmentShader;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline2D>("", sol::no_construction());
    v["SetVertexShader"] = &GraphicsPipeline2D::SetVertexShader;
    v["SetFragmentShader"] = &GraphicsPipeline2D::SetFragmentShader;
    v["Build"] = &GraphicsPipeline2D::Build;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline3D>("", sol::no_construction());
    v["SetVertexShader"] = &GraphicsPipeline3D::SetVertexShader;
    v["SetVertexInstancedShader"] =
        &GraphicsPipeline3D::SetVertexInstancedShader;
    v["SetFragmentShader"] = &GraphicsPipeline3D::SetFragmentShader;
    v["SetAnimation"] = &GraphicsPipeline3D::SetAnimation;
    v["Build"] = &GraphicsPipeline3D::build;
  }
  {
    auto v = lua.new_usertype<Draw2D>("", sol::no_construction());
    v["Draw"] = &Draw2D::Draw;
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["material"] = &Draw2D::material;
    v["Add"] = &Draw2D::Add;
    v["At"] = &Draw2D::At;
    v["Clear"] = &Draw2D::Clear;
  }
  {
    auto v = lua.new_usertype<Draw3D>("", sol::no_construction());
    v["position"] = &Draw3D::position;
    v["rotation"] = &Draw3D::rotation;
    v["scale"] = &Draw3D::scale;
    v["material"] = &Draw3D::material;
    v["model"] = &Draw3D::model;
    v["isDrawDepth"] = &Draw3D::isDrawDepth;
    v["Draw"] = &Draw3D::Draw;
    v["Add"] = &Draw3D::Add;
    v["At"] = &Draw3D::At;
    v["Clear"] = &Draw3D::Clear;
  }
  {
    auto v = lua.create_named("Random");
    v["GetRange"] = &Random::GetRange;
    v["GetIntRange"] = &Random::GetIntRange;
  }
  {
    auto v = lua.create_named("Window");
    v["GetName"] = &Window::GetName;
    v["Size"] = &Window::Size;
    v["Half"] = Window::Half;
    v["Resize"] = &Window::Resize;
    v["SetFullscreen"] = &Window::SetFullscreen;
    v["Rename"] = &Window::Rename;
    v["Resized"] = &Window::Resized;
  }
  {
    auto v = lua.create_named("Graphics");
    v["GetClearColor"] = &Graphics::GetClearColor;
    v["SetClearColor"] = &Graphics::SetClearColor;
    v["BindPipeline2D"] = &Graphics::BindPipeline2D;
    v["BindDefaultPipeline2D"] = &Graphics::BindDefaultPipeline2D;
    v["BindPipeline3D"] = &Graphics::BindPipeline3D;
    v["BindDefaultPipeline3D"] = &Graphics::BindDefaultPipeline3D;
    v["SetUniformData"] = &Graphics::SetUniformData;
    v["BeginTarget2D"] = &Graphics::BeginTarget2D;
    v["BeginTarget3D"] = &Graphics::BeginTarget3D;
    v["EndTarget"] = &Graphics::EndTarget;
  }
  {
    auto v = lua.create_named("Scene");
    v["GetCamera"] = &Scene::GetCamera;
    v["Size"] = &Scene::Size;
    v["Resize"] = &Scene::Resize;
    v["Half"] = &Scene::Half;
    v["Ratio"] = &Scene::Ratio;
    v["InvRatio"] = &Scene::InvRatio;
    v["DeltaTime"] = &Scene::DeltaTime;
    v["Change"] = &Scene::Change;
  }
  {
    auto v = lua.create_named("Collision");
    v["AABBvsAABB"] = &Collision::AABBvsAABB;
  }
  {
    auto v = lua.create_named("Keyboard");
    v["IsPressed"] = &Keyboard::IsPressed;
    v["IsReleased"] = &Keyboard::IsReleased;
    v["IsDown"] = &Keyboard::IsDown;
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
    v["SetRelative"] = &Mouse::SetRelative;
    v["IsRelative"] = &Mouse::IsRelative;
    v["IsPressed"] = &Mouse::IsPressed;
    v["IsReleased"] = &Mouse::IsReleased;
    v["IsDown"] = &Mouse::IsDown;
    v["GetPosition"] = &Mouse::GetPosition;
    v["GetPositionOnScene"] = &Mouse::GetPositionOnScene;
    v["SetPosition"] = &Mouse::SetPosition;
    v["SetPositionOnScene"] = &Mouse::SetPositionOnScene;
    v["GetScrollWheel"] = &Mouse::GetScrollWheel;
    v["HideCursor"] = &Mouse::HideCursor;
    v["LEFT"] = (int)Mouse::LEFT;
    v["RIGHT"] = (int)Mouse::RIGHT;
    v["MIDDLE"] = (int)Mouse::MIDDLE;
    v["X1"] = (int)Mouse::X1;
    v["X2"] = (int)Mouse::X2;
  }
  {
    auto v = lua.create_named("Gamepad");
    v["IsPressed"] = &GamePad::IsPressed;
    v["IsReleased"] = &GamePad::IsReleased;
    v["IsDown"] = &GamePad::IsDown;
    v["GetLeftStick"] = &GamePad::GetLeftStick;
    v["GetRightStick"] = &GamePad::GetRightStick;
    v["IsConnected"] = &GamePad::IsConnected;
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
    v["Sin0_1"] = [](float period, float t) {
      return Periodic::sin0_1(period, t);
    };
    v["Cos0_1"] = [](float period, float t) {
      return Periodic::cos0_1(period, t);
    };
  }
  {
    auto v = lua.create_named("Time");
    v["Seconds"] = &Time::Seconds;
    v["Milli"] = &Time::Milli;
  }
  {
    // logger
    auto v = lua.create_named("Logger");
    v["Verbose"] = [](std::string str) { Logger::Verbose("%s", str.data()); };
    v["Info"] = [](std::string str) { Logger::Info("%s", str.data()); };
    v["Error"] = [](std::string str) { Logger::Error("%s", str.data()); };
    v["Warn"] = [](std::string str) { Logger::Warn("%s", str.data()); };
    v["Critical"] = [](std::string str) { Logger::Critical("%s", str.data()); };
  }
  lua["Grid"] = [](int width, int height) -> Grid<int> {
    return Grid<int>(width, height);
  };
  {
    auto v = lua.new_usertype<Grid<int>>("", sol::no_construction());
    v["At"] = [](Grid<int> &g, int x, int y) { return g.at(x - 1, y - 1); };
    v["Set"] = [](Grid<int> &g, int x, int y, int v) {
      return g.at(x - 1, y - 1) = v;
    };
    v["Width"] = &Grid<int>::Width;
    v["Height"] = &Grid<int>::Height;
    v["Size"] = &Grid<int>::Size;
    v["Clear"] = &Grid<int>::Clear;
    v["Resize"] = &Grid<int>::Resize;
    v["Fill"] = [](Grid<int> &g, int value) {
      for (auto &i : g) {
        i = value;
      }
    };
  };
  lua["BFSGrid"] = [](const Grid<int> &g) { return BFSGrid(g); };
  {
    auto v = lua.new_usertype<BFSGrid>("", sol::no_construction());
    v["Width"] = &BFSGrid::Width;
    v["Height"] = &BFSGrid::Height;
    v["FindPath"] = [](BFSGrid &g, const glm::ivec2 &start,
                       const glm::ivec2 &end) {
      return g.FindPath({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["Trace"] = [](BFSGrid &g) {
      auto t = g.Trace();
      return glm::ivec2{t.x + 1, t.y + 1};
    };
    v["Traceable"] = &BFSGrid::Traceable;
    v["Reset"] = &BFSGrid::Reset;
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

void LuaScript::Update() { state["Update"](); }

void LuaScript::Draw() { state["Draw"](); }

} // namespace sinen