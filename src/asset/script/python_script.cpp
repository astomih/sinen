#include "script_backend.hpp"

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <graphics/graphics.hpp>
#include <math/graph/bfs_grid.hpp>
#include <math/graph/grid.hpp>
#include <math/math.hpp>
#include <physics/physics.hpp>
#include <platform/platform.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <SDL3/SDL.h>

namespace py = pybind11;

namespace sinen {
class PythonScript final : public IScriptBackend {
public:
  bool Initialize() override {
    py::initialize();
    pkbind::object_pool::initialize(65535);
    auto *callback = py_callbacks();
    callback->importfile = [](const char *path) -> char * {
      SDL_IOStream *io =
          (SDL_IOStream *)AssetIO::openAsIOStream(AssetType::Script, path);
      if (!io) {
        return nullptr;
      }
      size_t fileLength;
      void *load = SDL_LoadFile_IO(io, &fileLength, true);
      if (!load) {
        return nullptr;
      }
      return (char *)load;
    };
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

std::unique_ptr<IScriptBackend> ScriptBackend::CreatePython() {
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

  py::class_<glm::ivec2>(m, "Vec2i")
      .def(py::init<int, int>())
      .def(py::init<int>())
      .def_readwrite("x", &glm::ivec2::x)
      .def_readwrite("y", &glm::ivec2::y)
      .def(py::self + py::self)
      .def(py::self - py::self);

  py::class_<glm::ivec3>(m, "Vec3i")
      .def(py::init<int, int, int>())
      .def(py::init<int>())
      .def_readwrite("x", &glm::ivec3::x)
      .def_readwrite("y", &glm::ivec3::y)
      .def_readwrite("z", &glm::ivec3::z)
      .def(py::self + py::self)
      .def(py::self - py::self);

  // Texture
  py::class_<Texture>(m, "Texture")
      .def(py::init<>())
      .def("fill", &Texture::fill)
      .def("blend", &Texture::blend)
      .def("copy", &Texture::copy)
      .def("load", &Texture::load)
      .def("size", &Texture::size);

  // Material
  py::class_<Material>(m, "Material")
      .def(py::init<>())
      .def("append_texture", &Material::appendTexture)
      .def("clear", &Material::clear)
      .def("get_texture", &Material::getTexture,
           py::return_value_policy::reference);

  py::class_<RenderTexture>(m, "RenderTexture")
      .def(py::init<>())
      .def("create", &RenderTexture::create);

  py::class_<Music>(m, "Music")
      .def(py::init<>())
      .def("load", &Music::load)
      .def("play", &Music::play)
      .def("set_volume", &Music::setVolume);

  py::class_<Sound>(m, "Sound")
      .def(py::init<>())
      .def("load", &Sound::load)
      .def("play", &Sound::play)
      .def("set_volume", &Sound::setVolume)
      .def("set_pitch", &Sound::setPitch)
      .def("set_listener", &Sound::setListener)
      .def("set_position", &Sound::setPosition);

  py::class_<Camera>(m, "Camera")
      .def(py::init<>())
      .def("lookat", &Camera::lookat)
      .def("perspective", &Camera::perspective)
      .def("orthographic", &Camera::orthographic)
      .def("position", &Camera::getPosition)
      .def("target", &Camera::getTarget)
      .def("up", &Camera::getUp)
      .def("is_aabb_in_frustum", &Camera::isAABBInFrustum);

  py::class_<Camera2D>(m, "Camera2D")
      .def(py::init<>())
      .def("size", &Camera2D::size)
      .def("half", &Camera2D::half)
      .def("resize", &Camera2D::resize)
      .def("window_ratio", &Camera2D::windowRatio)
      .def("inv_window_ratio", &Camera2D::invWindowRatio);

  // Model
  py::class_<Model>(m, "Model")
      .def(py::init<>())
      .def("aabb", &Model::getAABB)
      .def("load", &Model::load)
      .def("load_sprite", &Model::loadSprite)
      .def("load_box", &Model::loadBox)
      .def("get_bone_uniform_data", &Model::getBoneUniformData)
      .def("play", &Model::play)
      .def("get_material", &Model::getMaterial)
      .def("update", &Model::update);

  // AABB
  py::class_<AABB>(m, "AABB")
      .def(py::init<>())
      .def_readwrite("min", &AABB::min)
      .def_readwrite("max", &AABB::max)
      .def("update_world", &AABB::updateWorld);

  // Timer
  py::class_<Timer>(m, "Timer")
      .def(py::init<>())
      .def("start", &Timer::start)
      .def("stop", &Timer::stop)
      .def("is_started", &Timer::isStarted)
      .def("set_time", &Timer::setTime)
      .def("check", &Timer::check);

  // Collider
  py::class_<Collider>(m, "Collider")
      .def(py::init<>())
      .def("get_position", &Collider::getPosition)
      .def("get_velocity", &Collider::getVelocity)
      .def("set_linear_velocity", &Collider::setLinearVelocity);

  // FileSystem
  py::class_<FileSystem>(m, "FileSystem")
      .def_static("enumerate_directory", &FileSystem::enumerateDirectory);

  // Script
  py::class_<Script>(m, "Script").def_static("load", &Script::load);

  // Uniform Data
  py::class_<UniformData>(m, "UniformData")
      .def(py::init<>())
      .def("add", &UniformData::add)
      .def("change", &UniformData::change);

  // Shader
  py::class_<Shader>(m, "Shader")
      .def(py::init<>())
      .def("load_vertex_shader", &Shader::loadVertexShader)
      .def("load_fragment_shader", &Shader::loadFragmentShader)
      .def("compile_and_load_vertex_shader",
           &Shader::compileAndLoadVertexShader)
      .def("compile_and_load_fragment_shader",
           &Shader::compileAndLoadFragmentShader);

  // Font
  py::class_<Font>(m, "Font")
      .def(py::init<>())
      // Overload Font::load
      .def("load", py::overload_cast<int>(&Font::load))
      .def("load", py::overload_cast<int, std::string_view>(&Font::load))
      .def("render_text", &Font::renderText)
      .def("resize", &Font::resize);
  // Color
  py::class_<Color>(m, "Color")
      .def(py::init<>())
      .def(py::init<float>())
      .def(py::init<float, float>())
      .def(py::init<float, float, float, float>())
      .def_readwrite("r", &Color::r)
      .def_readwrite("g", &Color::g)
      .def_readwrite("b", &Color::b)
      .def_readwrite("a", &Color::a);
  // Draw2D
  py::class_<Draw2D>(m, "Draw2D")
      .def(py::init<>())
      .def(py::init<Texture>())
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
      .def(py::init<Texture>())
      .def("add", &Draw3D::add)
      .def("at", &Draw3D::at)
      .def("clear", &Draw3D::clear)
      .def_readwrite("scale", &Draw3D::scale)
      .def_readwrite("position", &Draw3D::position)
      .def_readwrite("rotation", &Draw3D::rotation)
      .def_readwrite("material", &Draw3D::material)
      .def_readwrite("model", &Draw3D::model);

  // Rect
  py::class_<Rect>(m, "Rect")
      .def(py::init<>())
      .def(py::init<float, float, float, float>())
      .def(py::init<const glm::vec2 &, const glm::vec2 &>())
      .def_readwrite("x", &Rect::x)
      .def_readwrite("y", &Rect::y)
      .def_readwrite("width", &Rect::width)
      .def_readwrite("height", &Rect::height);

  // Transform
  py::class_<Transform>(m, "Transform")
      .def(py::init<>())
      .def_readwrite("position", &Transform::position)
      .def_readwrite("rotation", &Transform::rotation)
      .def_readwrite("scale", &Transform::scale);

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
             return g.findPath({start.x, start.y}, {end.x, end.y});
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
      .def("set_vertex_shader", &GraphicsPipeline2D::setVertexShader)
      .def("set_fragment_shader", &GraphicsPipeline2D::setFragmentShader)
      .def("build", &GraphicsPipeline2D::build);
  py::class_<GraphicsPipeline3D>(m, "GraphicsPipeline3D")
      .def(py::init<>())
      .def("set_vertex_shader", &GraphicsPipeline3D::setVertexShader)
      .def("set_fragment_shader", &GraphicsPipeline3D::setFragmentShader)
      .def("set_instanced", &GraphicsPipeline3D::setInstanced)
      .def("set_animation", &GraphicsPipeline3D::setAnimation)
      .def("build", &GraphicsPipeline3D::build);

  py::class_<Random>(m, "Random")
      .def_static("get_int_range", &Random::getIntRange)
      .def_static("get_float_range", &Random::getRange);

  py::class_<Window>(m, "Window")
      .def_static("name", &Window::getName)
      .def_static("size", &Window::size)
      .def_static("half", &Window::half)
      .def_static("resize", &Window::resize)
      .def_static("set_fullscreen", &Window::setFullscreen)
      .def_static("rename", &Window::rename)
      .def_static("resized", &Window::resized);

  py::class_<Physics>(m, "Physics")
      .def_static("create_box_collider", &Physics::createBoxCollider)
      .def_static("create_sphere_collider", &Physics::createSphereCollider)
      .def_static("cylinder_collider", &Physics::createCylinderCollider)
      .def_static("add_collider", &Physics::addCollider);

  py::class_<Graphics>(m, "Graphics")
      .def_static("draw2d", &Graphics::draw2D)
      .def_static("draw3d", &Graphics::draw3D)
      .def_static("draw_rect",
                  py::overload_cast<const Rect &, const Color &, float>(
                      &Graphics::drawRect))
      .def_static("draw_rect", py::overload_cast<const Rect &, const Color &>(
                                   &Graphics::drawRect))
      .def_static("draw_image",
                  py::overload_cast<const Texture &, const Rect &, float>(
                      &Graphics::drawImage))
      .def_static("draw_image",
                  py::overload_cast<const Texture &, const Rect &>(
                      &Graphics::drawImage))
      .def_static(
          "draw_text",
          py::overload_cast<const std::string &, const glm::vec2 &,
                            const Color &, float, float>(&Graphics::drawText))
      .def_static("draw_text",
                  py::overload_cast<const std::string &, const glm::vec2 &,
                                    const Color &, float>(&Graphics::drawText))
      .def_static("draw_text",
                  py::overload_cast<const std::string &, const glm::vec2 &,
                                    const Color &>(&Graphics::drawText))
      .def_static("draw_text",
                  py::overload_cast<const std::string &, const glm::vec2 &>(
                      &Graphics::drawText))
      .def_static("draw_model", &Graphics::drawModel)
      .def_static("draw_model_instanced", &Graphics::drawModelInstanced)
      .def_static("set_camera", &Graphics::setCamera)
      .def_static("get_camera", &Graphics::getCamera,
                  py::return_value_policy::reference)
      .def_static("set_camera2d", &Graphics::setCamera2D)
      .def_static("get_camera2d", &Graphics::getCamera2D,
                  py::return_value_policy::reference)
      .def_static("get_clear_color", &Graphics::getClearColor)
      .def_static("set_clear_color", &Graphics::setClearColor)
      .def_static("bind_pipeline2d", &Graphics::bindPipeline2D)
      .def_static("bind_default_pipeline2d", &Graphics::bindDefaultPipeline2D)
      .def_static("bind_pipeline3d", &Graphics::bindPipeline3D)
      .def_static("bind_default_pipeline3d", &Graphics::bindDefaultPipeline3D)
      .def_static("set_uniform_data", &Graphics::setUniformData)
      .def_static("set_render_target", &Graphics::setRenderTarget)
      .def_static("flush", &Graphics::flush)
      .def_static("readback_texture", &Graphics::readbackTexture);

  py::class_<Collision>(m, "Collision")
      .def_static("aabb_aabb", &Collision::AABBvsAABB);

  py::enum_<Keyboard::Code>(m, "Keyboard")
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
      .def("is_pressed", &Keyboard::isPressed)
      .def("is_released", &Keyboard::isReleased)
      .def("is_down", &Keyboard::isDown);

  py::enum_<Mouse::Code>(m, "Mouse")
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
      .def_static("is_pressed", &GamePad::isPressed)
      .def_static("is_released", &GamePad::isReleased)
      .def_static("is_down", &GamePad::isDown)
      .def_static("left_stick", &GamePad::getLeftStick)
      .def_static("right_stick", &GamePad::getRightStick)
      .def_static("is_connected", &GamePad::isConnected);

  py::class_<Periodic>(m, "Periodic")
      .def(py::init<>())
      .def_static("sin0_1", &Periodic::sin0_1)
      .def_static("cos0_1", &Periodic::cos0_1);

  py::class_<Time>(m, "Time")
      .def(py::init<>())
      .def_static("seconds", &Time::seconds)
      .def_static("milli", &Time::milli)
      .def_static("delta", &Time::deltaTime);

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