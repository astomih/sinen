#include "../../include/script.hpp"

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <logic/logic.hpp>
#include <math/graph/bfs_grid.hpp>
#include <math/graph/grid.hpp>
#include <math/math.hpp>
#include <platform/platform.hpp>
#include <render/render.hpp>

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

  void RunScene(const std::string_view source) override {
    py_exec(std::string(source).data(), "<string>", EXEC_MODE, nullptr);
  }

  void Update() override {
    py_exec("update()", "<string>", EVAL_MODE, nullptr);
  }
  void Draw() override { py_exec("draw()", "<string>", EVAL_MODE, nullptr); }
};

std::unique_ptr<IScript> Script::CreatePython() {
  return std::make_unique<PythonScript>();
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

  py::class_<RenderTexture>(m, "RenderTexture")
      .def(py::init<>())
      .def("create", &RenderTexture::create);

  py::class_<Music>(m, "Music")
      .def(py::init<>())
      .def("load", &Music::load)
      .def("play", &Music::play)
      .def("set_volume", &Music::set_volume);

  py::class_<Sound>(m, "Sound")
      .def(py::init<>())
      .def("load", &Sound::load)
      .def("play", &Sound::play)
      .def("set_volume", &Sound::set_volume)
      .def("set_pitch", &Sound::set_pitch)
      .def("set_listener", &Sound::set_listener)
      .def("set_position", &Sound::set_position);

  py::class_<Camera>(m, "Camera")
      .def(py::init<>())
      .def("lookat", &Camera::lookat)
      .def("perspective", &Camera::perspective)
      .def("orthographic", &Camera::orthographic)
      .def("position", &Camera::position)
      .def("target", &Camera::target)
      .def("up", &Camera::up)
      .def("is_aabb_in_frustum", &Camera::is_aabb_in_frustum);

  // Model
  py::class_<Model>(m, "Model")
      .def(py::init<>())
      .def("aabb", &Model::aabb)
      .def("load", &Model::load)
      .def("load_sprite", &Model::load_sprite)
      .def("load_box", &Model::load_box)
      .def("bone_uniform_data", &Model::bone_uniform_data)
      .def("play", &Model::play)
      .def("update", &Model::update);

  // AABB
  py::class_<AABB>(m, "AABB")
      .def(py::init<>())
      .def_readwrite("min", &AABB::min)
      .def_readwrite("max", &AABB::max)
      .def("update_world", &AABB::update_world);

  // Timer
  py::class_<Timer>(m, "Timer")
      .def(py::init<>())
      .def("start", &Timer::start)
      .def("stop", &Timer::stop)
      .def("is_started", &Timer::is_started)
      .def("set_time", &Timer::set_time)
      .def("check", &Timer::check);

  // Uniform Data
  py::class_<UniformData>(m, "UniformData")
      .def(py::init<>())
      .def("add", &UniformData::add)
      .def("change", &UniformData::change);

  // Shader
  py::class_<Shader>(m, "Shader")
      .def(py::init<>())
      .def("load_vertex_shader", &Shader::load_vertex_shader)
      .def("load_fragment_shader", &Shader::load_fragment_shader)
      .def("compile_and_load_vertex_shader",
           &Shader::compile_and_load_vertex_shader)
      .def("compile_and_load_fragment_shader",
           &Shader::compile_and_load_fragment_shader);

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
  // Draw2D
  py::class_<Draw2D>(m, "Draw2D")
      .def(py::init<>())
      .def(py::init<Texture>())
      .def("draw", &Draw2D::draw)
      .def("add", &Draw2D::add)
      .def("at", &Draw2D::at)
      .def("clear", &Draw2D::clear)
      .def_readwrite("scale", &Draw2D::scale)
      .def_readwrite("position", &Draw2D::position)
      .def_readwrite("rotation", &Draw2D::rotation)
      .def_readwrite("texture", &Draw2D::texture_handle)
      .def_readwrite("vertex_name", &Draw2D::vertex_name);

  // Draw3D
  py::class_<Draw3D>(m, "Draw3D")
      .def(py::init<>())
      .def(py::init<Texture>())
      .def("draw", &Draw3D::draw)
      .def("add", &Draw3D::add)
      .def("at", &Draw3D::at)
      .def("clear", &Draw3D::clear)
      .def_readwrite("scale", &Draw3D::scale)
      .def_readwrite("position", &Draw3D::position)
      .def_readwrite("rotation", &Draw3D::rotation)
      .def_readwrite("texture", &Draw3D::texture_handle)
      .def_readwrite("model", &Draw3D::model)
      .def_readwrite("is_draw_depth", &Draw3D::is_draw_depth);

  py::class_<Grid<int>>(m, "Grid")
      .def(py::init<int, int>())
      .def("at", &Grid<int>::at)
      .def("set",
           [](Grid<int> &g, int x, int y, int v) { return g.at(x, y) = v; })
      .def("width", &Grid<int>::width)
      .def("height", &Grid<int>::height)
      .def("size", &Grid<int>::size)
      .def("clear", &Grid<int>::clear)
      .def("resize", &Grid<int>::resize)
      .def("fill", [](Grid<int> &g, int value) {
        for (auto &i : g) {
          i = value;
        }
      });

  py::class_<BFSGrid>(m, "BFSGrid")
      .def(py::init<const Grid<int> &>())
      .def("width", &BFSGrid::width)
      .def("height", &BFSGrid::height)
      .def("find_path",
           [](BFSGrid &g, const glm::ivec2 &start, const glm::ivec2 &end) {
             return g.find_path({start.x, start.y}, {end.x, end.y});
           })
      .def("trace",
           [](BFSGrid &g) {
             auto t = g.trace();
             return glm::ivec2{t.x, t.y};
           })
      .def("traceable", &BFSGrid::traceable)
      .def("reset", &BFSGrid::reset);

  // submodules

  auto random = m.def_submodule("random");
  random.def("get_int_range", &Random::get_int_range);
  random.def("get_float_range", &Random::get_float_range);

  auto window = m.def_submodule("window");
  window.def("name", &Window::name);
  window.def("size", &Window::size);
  window.def("half", &Window::half);
  window.def("resize", &Window::resize);
  window.def("set_fullscreen", &Window::set_fullscreen);
  window.def("rename", &Window::rename);
  window.def("resized", &Window::resized);

  auto renderer = m.def_submodule("renderer");
  renderer.def("clear_color", &Renderer::clear_color);
  renderer.def("set_clear_color", &Renderer::set_clear_color);
  renderer.def("at_render_texture_user_data",
               &Renderer::at_render_texture_user_data);
  renderer.def("begin_pipeline2d", &Renderer::begin_pipeline2d);
  renderer.def("end_pipeline2d", &Renderer::end_pipeline2d);
  renderer.def("begin_pipeline3d", &Renderer::begin_pipeline3d);
  renderer.def("end_pipeline3d", &Renderer::end_pipeline3d);
  renderer.def("set_uniform_data", &Renderer::set_uniform_data);
  renderer.def("begin_render_texture2d", &Renderer::begin_render_texture2d);
  renderer.def("begin_render_texture3d", &Renderer::begin_render_texture3d);
  renderer.def("end_render_texture", &Renderer::end_render_texture);

  auto scene = m.def_submodule("scene");
  scene.def("camera", &Scene::camera);
  scene.def("size", &Scene::size);
  scene.def("resize", &Scene::resize);
  scene.def("half", &Scene::half);
  scene.def("ratio", &Scene::ratio);
  scene.def("inv_ratio", &Scene::inv_ratio);
  scene.def("delta_time", &Scene::delta_time);
  scene.def("change", &Scene::change);

  auto collision = m.def_submodule("collision");
  collision.def("aabb_aabb", &Collision::aabb_aabb);

  auto keyboard = m.def_submodule("keyboard");
  keyboard.def("is_pressed", &Keyboard::is_pressed);
  keyboard.def("is_released", &Keyboard::is_released);
  keyboard.def("is_down", &Keyboard::is_down);
  keyboard.def("UNKNOWN", []() -> int {
    return static_cast<int>(Keyboard::code::UNKNOWN);
  });
  keyboard.def("A",
               []() -> int { return static_cast<int>(Keyboard::code::A); });
  keyboard.def("B",
               []() -> int { return static_cast<int>(Keyboard::code::B); });
  keyboard.def("C",
               []() -> int { return static_cast<int>(Keyboard::code::C); });
  keyboard.def("D",
               []() -> int { return static_cast<int>(Keyboard::code::D); });
  keyboard.def("E",
               []() -> int { return static_cast<int>(Keyboard::code::E); });
  keyboard.def("F",
               []() -> int { return static_cast<int>(Keyboard::code::F); });
  keyboard.def("G",
               []() -> int { return static_cast<int>(Keyboard::code::G); });
  keyboard.def("H",
               []() -> int { return static_cast<int>(Keyboard::code::H); });
  keyboard.def("I",
               []() -> int { return static_cast<int>(Keyboard::code::I); });
  keyboard.def("J",
               []() -> int { return static_cast<int>(Keyboard::code::J); });
  keyboard.def("K",
               []() -> int { return static_cast<int>(Keyboard::code::K); });
  keyboard.def("L",
               []() -> int { return static_cast<int>(Keyboard::code::L); });
  keyboard.def("M",
               []() -> int { return static_cast<int>(Keyboard::code::M); });
  keyboard.def("N",
               []() -> int { return static_cast<int>(Keyboard::code::N); });
  keyboard.def("O",
               []() -> int { return static_cast<int>(Keyboard::code::O); });
  keyboard.def("P",
               []() -> int { return static_cast<int>(Keyboard::code::P); });
  keyboard.def("Q",
               []() -> int { return static_cast<int>(Keyboard::code::Q); });
  keyboard.def("R",
               []() -> int { return static_cast<int>(Keyboard::code::R); });
  keyboard.def("S",
               []() -> int { return static_cast<int>(Keyboard::code::S); });
  keyboard.def("T",
               []() -> int { return static_cast<int>(Keyboard::code::T); });
  keyboard.def("U",
               []() -> int { return static_cast<int>(Keyboard::code::U); });
  keyboard.def("V",
               []() -> int { return static_cast<int>(Keyboard::code::V); });
  keyboard.def("W",
               []() -> int { return static_cast<int>(Keyboard::code::W); });
  keyboard.def("X",
               []() -> int { return static_cast<int>(Keyboard::code::X); });
  keyboard.def("Y",
               []() -> int { return static_cast<int>(Keyboard::code::Y); });
  keyboard.def("Z",
               []() -> int { return static_cast<int>(Keyboard::code::Z); });
  keyboard.def("Key0",
               []() -> int { return static_cast<int>(Keyboard::code::Key0); });
  keyboard.def("Key1",
               []() -> int { return static_cast<int>(Keyboard::code::Key1); });
  keyboard.def("Key2",
               []() -> int { return static_cast<int>(Keyboard::code::Key2); });
  keyboard.def("Key3",
               []() -> int { return static_cast<int>(Keyboard::code::Key3); });
  keyboard.def("Key4",
               []() -> int { return static_cast<int>(Keyboard::code::Key4); });
  keyboard.def("Key5",
               []() -> int { return static_cast<int>(Keyboard::code::Key5); });
  keyboard.def("Key6",
               []() -> int { return static_cast<int>(Keyboard::code::Key6); });
  keyboard.def("Key7",
               []() -> int { return static_cast<int>(Keyboard::code::Key7); });
  keyboard.def("Key8",
               []() -> int { return static_cast<int>(Keyboard::code::Key8); });
  keyboard.def("Key9",
               []() -> int { return static_cast<int>(Keyboard::code::Key9); });
  keyboard.def("F1",
               []() -> int { return static_cast<int>(Keyboard::code::F1); });
  keyboard.def("F2",
               []() -> int { return static_cast<int>(Keyboard::code::F2); });
  keyboard.def("F3",
               []() -> int { return static_cast<int>(Keyboard::code::F3); });
  keyboard.def("F4",
               []() -> int { return static_cast<int>(Keyboard::code::F4); });
  keyboard.def("F5",
               []() -> int { return static_cast<int>(Keyboard::code::F5); });
  keyboard.def("F6",
               []() -> int { return static_cast<int>(Keyboard::code::F6); });
  keyboard.def("F7",
               []() -> int { return static_cast<int>(Keyboard::code::F7); });
  keyboard.def("F8",
               []() -> int { return static_cast<int>(Keyboard::code::F8); });
  keyboard.def("F9",
               []() -> int { return static_cast<int>(Keyboard::code::F9); });
  keyboard.def("F10",
               []() -> int { return static_cast<int>(Keyboard::code::F10); });
  keyboard.def("F11",
               []() -> int { return static_cast<int>(Keyboard::code::F11); });
  keyboard.def("F12",
               []() -> int { return static_cast<int>(Keyboard::code::F12); });
  keyboard.def("UP",
               []() -> int { return static_cast<int>(Keyboard::code::UP); });
  keyboard.def("DOWN",
               []() -> int { return static_cast<int>(Keyboard::code::DOWN); });
  keyboard.def("LEFT",
               []() -> int { return static_cast<int>(Keyboard::code::LEFT); });
  keyboard.def("RIGHT",
               []() -> int { return static_cast<int>(Keyboard::code::RIGHT); });
  keyboard.def("ESCAPE", []() -> int {
    return static_cast<int>(Keyboard::code::ESCAPE);
  });
  keyboard.def("SPACE",
               []() -> int { return static_cast<int>(Keyboard::code::SPACE); });
  keyboard.def("BACKSPACE", []() -> int {
    return static_cast<int>(Keyboard::code::BACKSPACE);
  });
  keyboard.def("TAB",
               []() -> int { return static_cast<int>(Keyboard::code::TAB); });
  keyboard.def("RETURN", []() -> int {
    return static_cast<int>(Keyboard::code::RETURN);
  });
  keyboard.def("LSHIFT", []() -> int {
    return static_cast<int>(Keyboard::code::LSHIFT);
  });
  keyboard.def("RSHIFT", []() -> int {
    return static_cast<int>(Keyboard::code::RSHIFT);
  });
  keyboard.def("LCTRL",
               []() -> int { return static_cast<int>(Keyboard::code::LCTRL); });
  keyboard.def("RCTRL",
               []() -> int { return static_cast<int>(Keyboard::code::RCTRL); });
  keyboard.def("LALT",
               []() -> int { return static_cast<int>(Keyboard::code::LALT); });

  auto mouse = m.def_submodule("mouse");
  mouse.def("set_relative", &Mouse::set_relative);
  mouse.def("is_relative", &Mouse::is_relative);
  mouse.def("is_pressed", &Mouse::is_pressed);
  mouse.def("is_released", &Mouse::is_released);
  mouse.def("is_down", &Mouse::is_down);
  mouse.def("position", &Mouse::get_position);
  mouse.def("position_on_scene", &Mouse::get_position_on_scene);
  mouse.def("set_position", &Mouse::set_position);
  mouse.def("set_position_on_scene", &Mouse::set_position_on_scene);
  mouse.def("scroll_wheel", &Mouse::get_scroll_wheel);
  mouse.def("hide_cursor", &Mouse::hide_cursor);
  mouse.def("LEFT",
            []() -> int { return static_cast<int>(Mouse::code::LEFT); });
  mouse.def("RIGHT",
            []() -> int { return static_cast<int>(Mouse::code::RIGHT); });
  mouse.def("MIDDLE",
            []() -> int { return static_cast<int>(Mouse::code::MIDDLE); });
  mouse.def("X1", []() -> int { return static_cast<int>(Mouse::code::X1); });
  mouse.def("X2", []() -> int { return static_cast<int>(Mouse::code::X2); });
  auto gamepad = m.def_submodule("gamepad");
  gamepad.def("is_pressed", &GamePad::is_pressed);
  gamepad.def("is_released", &GamePad::is_released);
  gamepad.def("is_down", &GamePad::is_down);
  gamepad.def("left_stick", &GamePad::get_left_stick);
  gamepad.def("right_stick", &GamePad::get_right_stick);
  gamepad.def("is_connected", &GamePad::is_connected);
  gamepad.def("INVALID",
              []() -> int { return static_cast<int>(GamePad::code::INVALID); });
  gamepad.def("A", []() -> int { return static_cast<int>(GamePad::code::A); });
  gamepad.def("B", []() -> int { return static_cast<int>(GamePad::code::B); });
  gamepad.def("X", []() -> int { return static_cast<int>(GamePad::code::X); });
  gamepad.def("Y", []() -> int { return static_cast<int>(GamePad::code::Y); });
  gamepad.def("BACK",
              []() -> int { return static_cast<int>(GamePad::code::BACK); });
  gamepad.def("GUIDE",
              []() -> int { return static_cast<int>(GamePad::code::GUIDE); });
  gamepad.def("START",
              []() -> int { return static_cast<int>(GamePad::code::START); });
  gamepad.def("LEFTSTICK", []() -> int {
    return static_cast<int>(GamePad::code::LEFTSTICK);
  });
  gamepad.def("RIGHTSTICK", []() -> int {
    return static_cast<int>(GamePad::code::RIGHTSTICK);
  });
  gamepad.def("LEFTSHOULDER", []() -> int {
    return static_cast<int>(GamePad::code::LEFTSHOULDER);
  });
  gamepad.def("RIGHTSHOULDER", []() -> int {
    return static_cast<int>(GamePad::code::RIGHTSHOULDER);
  });
  gamepad.def("DPAD_UP",
              []() -> int { return static_cast<int>(GamePad::code::DPAD_UP); });
  gamepad.def("DPAD_DOWN", []() -> int {
    return static_cast<int>(GamePad::code::DPAD_DOWN);
  });
  gamepad.def("DPAD_LEFT", []() -> int {
    return static_cast<int>(GamePad::code::DPAD_LEFT);
  });
  gamepad.def("DPAD_RIGHT", []() -> int {
    return static_cast<int>(GamePad::code::DPAD_RIGHT);
  });
  gamepad.def("MISC1",
              []() -> int { return static_cast<int>(GamePad::code::MISC1); });
  gamepad.def("PADDLE1",
              []() -> int { return static_cast<int>(GamePad::code::PADDLE1); });
  gamepad.def("PADDLE2",
              []() -> int { return static_cast<int>(GamePad::code::PADDLE2); });
  gamepad.def("PADDLE3",
              []() -> int { return static_cast<int>(GamePad::code::PADDLE3); });
  gamepad.def("PADDLE4",
              []() -> int { return static_cast<int>(GamePad::code::PADDLE4); });
  gamepad.def("TOUCHPAD", []() -> int {
    return static_cast<int>(GamePad::code::TOUCHPAD);
  });

  auto periodic = m.def_submodule("periodic");
  periodic.def("sin0_1", [](float period, float t) {
    return Periodic::sin0_1(period, t);
  });
  periodic.def("cos0_1", [](float period, float t) {
    return Periodic::cos0_1(period, t);
  });

  auto time = m.def_submodule("time");
  time.def("seconds", &Time::seconds);
  time.def("milli", &Time::milli);

  auto logger = m.def_submodule("logger");
  logger.def("verbose",
             [](std::string str) { Logger::verbose("%s", str.data()); });
  logger.def("debug", [](std::string str) { Logger::debug("%s", str.data()); });
  logger.def("info", [](std::string str) { Logger::info("%s", str.data()); });
  logger.def("error", [](std::string str) { Logger::error("%s", str.data()); });
  logger.def("warn", [](std::string str) { Logger::warn("%s", str.data()); });
  logger.def("critical",
             [](std::string str) { Logger::critical("%s", str.data()); });
}
} // namespace sinen