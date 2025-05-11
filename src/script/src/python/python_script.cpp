#include "../../include/script.hpp"

#include <asset/asset.hpp>
#include <core/core.hpp>
#include <logic/logic.hpp>
#include <math/math.hpp>
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