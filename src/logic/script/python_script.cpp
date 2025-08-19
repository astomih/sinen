#include "script.hpp"

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <graphics/graphics.hpp>
#include <logic/logic.hpp>
#include <math/graph/bfs_grid.hpp>
#include <math/graph/grid.hpp>
#include <math/math.hpp>
#include <platform/platform.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace sinen {
class PythonScript final : public IScript {
public:
  bool Initialize() override {
    py::initialize();
    return true;
  }
  void Finalize() override { py::finalize(); }

  py_GlobalRef m;
  py_GlobalRef updateRef;
  py_GlobalRef drawRef;
  void RunScene(const std::string_view source) override {
    if (!py_exec(std::string(source).data(), "<string>", EXEC_MODE, nullptr)) {
      py_printexc();
    }
    m = py_getmodule("__main__");
  }

  void Update() override {
    py_getattr(m, py_name("update"));
    updateRef = py_retval();
    py_push(updateRef);
    py_pushnil();
    py_vectorcall(0, 0);
  }
  void Draw() override {
    py_getattr(m, py_name("draw"));
    drawRef = py_retval();
    py_push(drawRef);
    py_pushnil();
    py_vectorcall(0, 0);
  }
};

std::unique_ptr<IScript> Script::CreatePython() {
  return std::make_unique<PythonScript>();
}

PYBIND11_EMBEDDED_MODULE(sinen, m) {
  py::class_<glm::vec3>(m, "Vec3")
      .def(py::init<>())
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
      .def("fill_color", &Texture::FillColor)
      .def("blend_color", &Texture::BlendColor)
      .def("copy", &Texture::Copy)
      .def("load", &Texture::Load)
      .def("size", &Texture::Size);

  // Material
  py::class_<Material>(m, "Material")
      .def(py::init<>())
      .def("append_texture", &Material::AppendTexture)
      .def("clear", &Material::Clear)
      .def("get_texture", &Material::GetTexture,
           py::return_value_policy::reference);

  py::class_<RenderTexture>(m, "RenderTexture")
      .def(py::init<>())
      .def("create", &RenderTexture::Create);

  py::class_<Music>(m, "Music")
      .def(py::init<>())
      .def("load", &Music::Load)
      .def("play", &Music::Play)
      .def("set_volume", &Music::SetVolume);

  py::class_<Sound>(m, "Sound")
      .def(py::init<>())
      .def("load", &Sound::Load)
      .def("play", &Sound::Play)
      .def("set_volume", &Sound::SetVolume)
      .def("set_pitch", &Sound::SetPitch)
      .def("set_listener", &Sound::SetListener)
      .def("set_position", &Sound::SetPosition);

  py::class_<Camera>(m, "Camera")
      .def(py::init<>())
      .def("lookat", &Camera::LookAt)
      .def("perspective", &Camera::Perspective)
      .def("orthographic", &Camera::Orthographic)
      .def("position", &Camera::GetPosition)
      .def("target", &Camera::GetTarget)
      .def("up", &Camera::GetUp)
      .def("is_aabb_in_frustum", &Camera::IsAABBInFrustum);

  // Model
  py::class_<Model>(m, "Model")
      .def(py::init<>())
      .def("aabb", &Model::GetAABB)
      .def("load", &Model::Load)
      .def("load_sprite", &Model::LoadSprite)
      .def("load_box", &Model::LoadBox)
      .def("bone_uniform_data", &Model::GetBoneUniformData)
      .def("play", &Model::Play)
      .def("update", &Model::Update);

  // AABB
  py::class_<AABB>(m, "AABB")
      .def(py::init<>())
      .def_readwrite("min", &AABB::min)
      .def_readwrite("max", &AABB::max)
      .def("update_world", &AABB::update_world);

  // Timer
  py::class_<Timer>(m, "Timer")
      .def(py::init<>())
      .def("start", &Timer::Start)
      .def("stop", &Timer::Stop)
      .def("is_started", &Timer::IsStarted)
      .def("set_time", &Timer::SetTime)
      .def("check", &Timer::Check);

  // Uniform Data
  py::class_<UniformData>(m, "UniformData")
      .def(py::init<>())
      .def("add", &UniformData::Add)
      .def("change", &UniformData::Change);

  // Shader
  py::class_<Shader>(m, "Shader")
      .def(py::init<>())
      .def("load_vertex_shader", &Shader::LoadVertexShader)
      .def("load_fragment_shader", &Shader::LoadFragmentShader)
      .def("compile_and_load_vertex_shader",
           &Shader::CompileAndLoadVertexShader)
      .def("compile_and_load_fragment_shader",
           &Shader::CompileAndLoadFragmentShader);

  // Font
  py::class_<Font>(m, "Font")
      .def(py::init<>())
      // Overload Font::load
      .def("load", py::overload_cast<int>(&Font::Load))
      .def("load", py::overload_cast<int, std::string_view>(&Font::Load))
      .def("render_text", &Font::RenderText)
      .def("resize", &Font::Resize);
  // Color
  py::class_<Color>(m, "Color")
      .def(py::init<>())
      .def(py::init<float, float, float, float>())
      .def_readwrite("r", &Color::r)
      .def_readwrite("g", &Color::g)
      .def_readwrite("b", &Color::b)
      .def_readwrite("a", &Color::a);
  // Draw2D
  py::class_<Draw2D>(m, "Draw2D")
      .def(py::init<>())
      .def(py::init<Texture>())
      .def("add", &Draw2D::Add)
      .def("at", &Draw2D::At)
      .def("clear", &Draw2D::Clear)
      .def_readwrite("scale", &Draw2D::scale)
      .def_readwrite("position", &Draw2D::position)
      .def_readwrite("rotation", &Draw2D::rotation)
      .def_readwrite("material", &Draw2D::material);

  // Draw3D
  py::class_<Draw3D>(m, "Draw3D")
      .def(py::init<>())
      .def("add", &Draw3D::Add)
      .def("at", &Draw3D::At)
      .def("clear", &Draw3D::Clear)
      .def_readwrite("scale", &Draw3D::scale)
      .def_readwrite("position", &Draw3D::position)
      .def_readwrite("rotation", &Draw3D::rotation)
      .def_readwrite("material", &Draw3D::material)
      .def_readwrite("model", &Draw3D::model)
      .def_readwrite("is_draw_depth", &Draw3D::isDrawDepth);

  py::class_<Grid<int>>(m, "Grid")
      .def(py::init<int, int>())
      .def("at", &Grid<int>::at)
      .def("set",
           [](Grid<int> &g, int x, int y, int v) { return g.at(x, y) = v; })
      .def("width", &Grid<int>::Width)
      .def("height", &Grid<int>::Height)
      .def("size", &Grid<int>::Size)
      .def("clear", &Grid<int>::Clear)
      .def("resize", &Grid<int>::Resize)
      .def("fill", [](Grid<int> &g, int value) {
        for (auto &i : g) {
          i = value;
        }
      });

  py::class_<BFSGrid>(m, "BFSGrid")
      .def(py::init<const Grid<int> &>())
      .def("width", &BFSGrid::Width)
      .def("height", &BFSGrid::Height)
      .def("find_path",
           [](BFSGrid &g, const glm::ivec2 &start, const glm::ivec2 &end) {
             return g.FindPath({start.x, start.y}, {end.x, end.y});
           })
      .def("trace",
           [](BFSGrid &g) {
             auto t = g.Trace();
             return glm::ivec2{t.x, t.y};
           })
      .def("traceable", &BFSGrid::Traceable)
      .def("reset", &BFSGrid::Reset);

  py::class_<GraphicsPipeline2D>(m, "GraphicsPipeline2D")
      .def(py::init<>())
      .def("set_vertex_shader", &GraphicsPipeline2D::SetVertexShader)
      .def("set_fragment_shader", &GraphicsPipeline2D::SetFragmentShader)
      .def("build", &GraphicsPipeline2D::Build);
  py::class_<GraphicsPipeline3D>(m, "GraphicsPipeline3D")
      .def(py::init<>())
      .def("set_vertex_shader", &GraphicsPipeline3D::SetVertexShader)
      .def("set_vertex_instanced_shader",
           &GraphicsPipeline3D::SetVertexInstancedShader)
      .def("set_fragment_shader", &GraphicsPipeline3D::SetFragmentShader)
      .def("set_animation", &GraphicsPipeline3D::SetAnimation)
      .def("build", &GraphicsPipeline3D::build);

  py::class_<Random>(m, "Random")
      .def_static("get_int_range", &Random::GetIntRange)
      .def_static("get_float_range", &Random::GetRange);

  py::class_<Window>(m, "Window")
      .def_static("name", &Window::GetName)
      .def_static("size", &Window::Size)
      .def_static("half", &Window::Half)
      .def_static("resize", &Window::Resize)
      .def_static("set_fullscreen", &Window::SetFullscreen)
      .def_static("rename", &Window::Rename)
      .def_static("resized", &Window::Resized);

  py::class_<Graphics>(m, "Graphics")
      .def_static("clear_color", &Graphics::GetClearColor)
      .def_static("set_clear_color", &Graphics::SetClearColor)
      .def_static("at_render_texture_user_data",
                  &Graphics::at_render_texture_user_data)
      .def_static("bind_pipeline2d", &Graphics::BindPipeline2D)
      .def_static("bind_default_pipeline2d", &Graphics::BindDefaultPipeline2D)
      .def_static("bind_pipeline3d", &Graphics::BindPipeline3D)
      .def_static("bind_default_pipeline3d", &Graphics::BindDefaultPipeline3D)
      .def_static("draw2d", &Graphics::Draw2D)
      .def_static("draw3d", &Graphics::Draw3D)
      .def_static("set_uniform_data", &Graphics::SetUniformData)
      .def_static("begin_target2d", &Graphics::BeginTarget2D)
      .def_static("begin_target3d", &Graphics::BeginTarget3D)
      .def_static("end_target", &Graphics::EndTarget);

  py::class_<Scene>(m, "Scene")
      .def(py::init<>())
      .def_static("camera", &Scene::GetCamera,
                  py::return_value_policy::reference)
      .def_static("size", &Scene::Size)
      .def_static("resize", &Scene::Resize)
      .def_static("half", &Scene::Half)
      .def_static("ratio", &Scene::Ratio)
      .def_static("inv_ratio", &Scene::InvRatio)
      .def_static("delta_time", &Scene::DeltaTime)
      .def_static("change", &Scene::Change);

  py::class_<Collision>(m, "Collision")
      .def_static("aabb_aabb", &Collision::AABBvsAABB);

  py::enum_<Keyboard::code>(m, "Keyboard")
      .value("A", Keyboard::A)
      .value("B", Keyboard::B)
      .value("C", Keyboard::C)
      .value("D", Keyboard::D)
      .value("E", Keyboard::E)
      .value("F", Keyboard::F)
      .value("G", Keyboard::G)
      .value("H", Keyboard::H)
      .value("I", Keyboard::I)
      .value("J", Keyboard::J)
      .value("K", Keyboard::K)
      .value("L", Keyboard::L)
      .value("M", Keyboard::M)
      .value("N", Keyboard::N)
      .value("O", Keyboard::O)
      .value("P", Keyboard::P)
      .value("Q", Keyboard::Q)
      .value("R", Keyboard::R)
      .value("S", Keyboard::S)
      .value("T", Keyboard::T)
      .value("U", Keyboard::U)
      .value("V", Keyboard::V)
      .value("W", Keyboard::W)
      .value("X", Keyboard::X)
      .value("Y", Keyboard::Y)
      .value("Z", Keyboard::Z)
      .value("Key0", Keyboard::Key0)
      .value("Key1", Keyboard::Key1)
      .value("Key2", Keyboard::Key2)
      .value("Key3", Keyboard::Key3)
      .value("Key4", Keyboard::Key4)
      .value("Key5", Keyboard::Key5)
      .value("Key6", Keyboard::Key6)
      .value("Key7", Keyboard::Key7)
      .value("Key8", Keyboard::Key8)
      .value("Key9", Keyboard::Key9)
      .value("F1", Keyboard::F1)
      .value("F2", Keyboard::F2)
      .value("F3", Keyboard::F3)
      .value("F4", Keyboard::F4)
      .value("F5", Keyboard::F5)
      .value("F6", Keyboard::F6)
      .value("F7", Keyboard::F7)
      .value("F8", Keyboard::F8)
      .value("F9", Keyboard::F9)
      .value("F10", Keyboard::F10)
      .value("F11", Keyboard::F11)
      .value("F12", Keyboard::F12)
      .value("UP", Keyboard::UP)
      .value("DOWN", Keyboard::DOWN)
      .value("LEFT", Keyboard::LEFT)
      .value("RIGHT", Keyboard::RIGHT)
      .value("ESCAPE", Keyboard::ESCAPE)
      .value("SPACE", Keyboard::SPACE)
      .value("BACKSPACE", Keyboard::BACKSPACE)
      .value("TAB", Keyboard::TAB)
      .value("RETURN", Keyboard::RETURN)
      .value("LSHIFT", Keyboard::LSHIFT)
      .value("RSHIFT", Keyboard::RSHIFT)
      .value("LCTRL", Keyboard::LCTRL)
      .value("RCTRL", Keyboard::RCTRL)
      .value("LALT", Keyboard::LALT)
      .def(py::init<>())
      .def("is_pressed", &Keyboard::IsPressed)
      .def("is_released", &Keyboard::IsReleased)
      .def("is_down", &Keyboard::IsDown);

  py::enum_<Mouse::code>(m, "Mouse")
      .value("LEFT", Mouse::LEFT)
      .value("RIGHT", Mouse::RIGHT)
      .value("MIDDLE", Mouse::MIDDLE)
      .value("X1", Mouse::X1)
      .value("X2", Mouse::X2)
      .def_static("is_pressed", &Mouse::IsPressed)
      .def_static("is_released", &Mouse::IsReleased)
      .def_static("is_down", &Mouse::IsDown)
      .def_static("position", &Mouse::GetPosition)
      .def_static("position_on_scene", &Mouse::GetPositionOnScene)
      .def_static("set_position", &Mouse::SetPosition)
      .def_static("set_position_on_scene", &Mouse::SetPositionOnScene)
      .def_static("scroll_wheel", &Mouse::GetScrollWheel)
      .def_static("hide_cursor", &Mouse::HideCursor);

  py::enum_<GamePad::code>(m, "GamePad")
      .value("INVALID", GamePad::INVALID)
      .value("A", GamePad::A)
      .value("B", GamePad::B)
      .value("X", GamePad::X)
      .value("Y", GamePad::Y)
      .value("BACK", GamePad::BACK)
      .value("GUIDE", GamePad::GUIDE)
      .value("START", GamePad::START)
      .value("LEFTSTICK", GamePad::LEFTSTICK)
      .value("RIGHTSTICK", GamePad::RIGHTSTICK)
      .value("LEFTSHOULDER", GamePad::LEFTSHOULDER)
      .value("RIGHTSHOULDER", GamePad::RIGHTSHOULDER)
      .value("DPAD_UP", GamePad::DPAD_UP)
      .value("DPAD_DOWN", GamePad::DPAD_DOWN)
      .value("DPAD_LEFT", GamePad::DPAD_LEFT)
      .value("DPAD_RIGHT", GamePad::DPAD_RIGHT)
      .value("MISC1", GamePad::MISC1)
      .value("PADDLE1", GamePad::PADDLE1)
      .value("PADDLE2", GamePad::PADDLE2)
      .value("PADDLE3", GamePad::PADDLE3)
      .value("PADDLE4", GamePad::PADDLE4)
      .value("TOUCHPAD", GamePad::TOUCHPAD)
      .def(py::init<>())
      .def_static("is_pressed", &GamePad::IsPressed)
      .def_static("is_released", &GamePad::IsReleased)
      .def_static("is_down", &GamePad::IsDown)
      .def_static("left_stick", &GamePad::GetLeftStick)
      .def_static("right_stick", &GamePad::GetRightStick)
      .def_static("is_connected", &GamePad::IsConnected);

  py::class_<Periodic>(m, "Periodic")
      .def(py::init<>())
      .def_static("sin0_1", &Periodic::sin0_1)
      .def_static("cos0_1", &Periodic::cos0_1);

  py::class_<Time>(m, "Time")
      .def(py::init<>())
      .def_static("seconds", &Time::Seconds)
      .def_static("milli", &Time::Milli);

  py::class_<Logger>(m, "Logger")
      .def_static("verbose",
                  [](const std::string &str) { Logger::Verbose(str); })
      .def_static("debug", [](const std::string &str) { Logger::Debug(str); })
      .def_static("info", [](const std::string &str) { Logger::Info(str); })
      .def_static("error", [](const std::string &str) { Logger::Error(str); })
      .def_static("warn", [](const std::string &str) { Logger::Warn(str); })
      .def_static("critical",
                  [](const std::string &str) { Logger::Critical(str); });
}
} // namespace sinen