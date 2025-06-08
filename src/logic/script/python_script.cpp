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

  void RunScene(const std::string_view source) override {
    if (!py_exec(std::string(source).data(), "<string>", EXEC_MODE, nullptr)) {
      py_printexc();
    }
  }

  void Update() override {
    if (!py_exec("update()", "<string>", EVAL_MODE, nullptr)) {
      py_printexc();
    }
  }
  void Draw() override { py_exec("draw()", "<string>", EVAL_MODE, nullptr); }
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
      .def("fill_color", &Texture::fill_color)
      .def("blend_color", &Texture::blend_color)
      .def("copy", &Texture::copy)
      .def("load", &Texture::load)
      .def("size", &Texture::size);

  // Material
  py::class_<Material>(m, "Material")
      .def(py::init<>())
      .def("append", &Material::append)
      .def("clear", &Material::clear)
      .def("get_texture", &Material::get_texture,
           py::return_value_policy::reference);

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
      // Overload Font::load
      .def("load", py::overload_cast<int>(&Font::load))
      .def("load", py::overload_cast<int, std::string_view>(&Font::load_from_file))
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
      .def("draw", &Draw2D::draw)
      .def("add", &Draw2D::add)
      .def("at", &Draw2D::at)
      .def("clear", &Draw2D::clear)
      .def_readwrite("scale", &Draw2D::scale)
      .def_readwrite("position", &Draw2D::position)
      .def_readwrite("rotation", &Draw2D::rotation)
      .def_readwrite("material", &Draw2D::material);

  // Draw3D
  py::class_<Draw3D>(m, "Draw3D")
      .def(py::init<>())
      .def("draw", &Draw3D::draw)
      .def("add", &Draw3D::add)
      .def("at", &Draw3D::at)
      .def("clear", &Draw3D::clear)
      .def_readwrite("scale", &Draw3D::scale)
      .def_readwrite("position", &Draw3D::position)
      .def_readwrite("rotation", &Draw3D::rotation)
      .def_readwrite("material", &Draw3D::material)
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

  py::class_<GraphicsPipeline2D>(m, "GraphicsPipeline2D")
      .def(py::init<>())
      .def("set_vertex_shader", &GraphicsPipeline2D::set_vertex_shader)
      .def("set_fragment_shader", &GraphicsPipeline2D::set_fragment_shader)
      .def("build", &GraphicsPipeline2D::build);
  py::class_<GraphicsPipeline3D>(m, "GraphicsPipeline3D")
      .def(py::init<>())
      .def("set_vertex_shader", &GraphicsPipeline3D::set_vertex_shader)
      .def("set_vertex_instanced_shader",
           &GraphicsPipeline3D::set_vertex_instanced_shader)
      .def("set_fragment_shader", &GraphicsPipeline3D::set_fragment_shader)
      .def("set_animation", &GraphicsPipeline3D::set_animation)
      .def("build", &GraphicsPipeline3D::build);

  py::class_<Random>(m, "Random")
      .def_static("get_int_range", &Random::get_int_range)
      .def_static("get_float_range", &Random::get_float_range);

  py::class_<Window>(m, "Window")
      .def_static("name", &Window::name)
      .def_static("size", &Window::size)
      .def_static("half", &Window::half)
      .def_static("resize", &Window::resize)
      .def_static("set_fullscreen", &Window::set_fullscreen)
      .def_static("rename", &Window::rename)
      .def_static("resized", &Window::resized);

  py::class_<Graphics>(m, "Graphics")
      .def_static("clear_color", &Graphics::clear_color)
      .def_static("set_clear_color", &Graphics::set_clear_color)
      .def_static("at_render_texture_user_data",
                  &Graphics::at_render_texture_user_data)
      .def_static("bind_pipeline2d", &Graphics::bind_pipeline2d)
      .def_static("bind_default_pipeline2d", &Graphics::bind_default_pipeline2d)
      .def_static("bind_pipeline3d", &Graphics::bind_pipeline3d)
      .def_static("bind_default_pipeline3d", &Graphics::bind_default_pipeline3d)
      .def_static("set_uniform_data", &Graphics::set_uniform_data)
      .def_static("begin_target2d", &Graphics::begin_target2d)
      .def_static("begin_target3d", &Graphics::begin_target3d)
      .def_static("end_target", &Graphics::end_target);

  py::class_<Scene>(m, "Scene")
      .def(py::init<>())
      .def_static("camera", &Scene::camera, py::return_value_policy::reference)
      .def_static("size", &Scene::size)
      .def_static("resize", &Scene::resize)
      .def_static("half", &Scene::half)
      .def_static("ratio", &Scene::ratio)
      .def_static("inv_ratio", &Scene::inv_ratio)
      .def_static("delta_time", &Scene::delta_time)
      .def_static("change", &Scene::change);

  py::class_<Collision>(m, "Collision")
      .def_static("aabb_aabb", &Collision::aabb_aabb);

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
      .def("is_pressed", &Keyboard::is_pressed)
      .def("is_released", &Keyboard::is_released)
      .def("is_down", &Keyboard::is_down);

  py::enum_<Mouse::code>(m, "Mouse")
      .value("LEFT", Mouse::LEFT)
      .value("RIGHT", Mouse::RIGHT)
      .value("MIDDLE", Mouse::MIDDLE)
      .value("X1", Mouse::X1)
      .value("X2", Mouse::X2)
      .def_static("is_pressed", &Mouse::is_pressed)
      .def_static("is_released", &Mouse::is_released)
      .def_static("is_down", &Mouse::is_down)
      .def_static("position", &Mouse::get_position)
      .def_static("position_on_scene", &Mouse::get_position_on_scene)
      .def_static("set_position", &Mouse::set_position)
      .def_static("set_position_on_scene", &Mouse::set_position_on_scene)
      .def_static("scroll_wheel", &Mouse::get_scroll_wheel)
      .def_static("hide_cursor", &Mouse::hide_cursor);

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
      .def_static("is_pressed", &GamePad::is_pressed)
      .def_static("is_released", &GamePad::is_released)
      .def_static("is_down", &GamePad::is_down)
      .def_static("left_stick", &GamePad::get_left_stick)
      .def_static("right_stick", &GamePad::get_right_stick)
      .def_static("is_connected", &GamePad::is_connected);

  py::class_<Periodic>(m, "Periodic")
      .def(py::init<>())
      .def_static("sin0_1", &Periodic::sin0_1)
      .def_static("cos0_1", &Periodic::cos0_1);

  py::class_<Time>(m, "Time")
      .def(py::init<>())
      .def_static("seconds", &Time::seconds)
      .def_static("milli", &Time::milli);

  py::class_<Logger>(m, "Logger")
      .def_static("verbose",
                  [](const std::string &str) { Logger::verbose(str); })
      .def_static("debug", [](const std::string &str) { Logger::debug(str); })
      .def_static("info", [](const std::string &str) { Logger::info(str); })
      .def_static("error", [](const std::string &str) { Logger::error(str); })
      .def_static("warn", [](const std::string &str) { Logger::warn(str); })
      .def_static("critical",
                  [](const std::string &str) { Logger::critical(str); });
}
} // namespace sinen