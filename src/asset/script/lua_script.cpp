#include "core/allocator/global_allocator.hpp"
#include "core/data/array.hpp"
#include "core/data/ptr.hpp"
#include "core/data/string.hpp"
#include "glm/ext/vector_float4.hpp"
#include "math/color/color.hpp"
#include "math/transform/transform.hpp"
#include "script_backend.hpp"

#include "math/graph/bfs_grid.hpp"
#include "math/graph/grid.hpp"
#include "platform/input/gamepad.hpp"
#include "platform/input/keyboard.hpp"
#include "platform/input/mouse.hpp"
#include "platform/window/window.hpp"
#include <asset/asset.hpp>
#include <core/core.hpp>
#include <core/io/arguments.hpp>
#include <core/io/file_system.hpp>
#include <format>
#include <glm/glm.hpp>
#include <graphics/graphics.hpp>
#include <main_system.hpp>
#include <math/math.hpp>
#include <physics/physics.hpp>
#include <string>

#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include <sol/sol.hpp>

namespace sinen {

auto alloc = [](void *ud, void *ptr, size_t osize, size_t nsize) -> void * {
  (void)ud;
  // free
  if (nsize == 0) {
    if (ptr)
      sinen::GlobalAllocator::get()->deallocate(ptr, osize);
    return nullptr;
  }

  if (ptr == nullptr) {
    return sinen::GlobalAllocator::get()->allocate(nsize);
  }

  void *nptr = sinen::GlobalAllocator::get()->allocate(nsize);
  if (!nptr) {
    return nullptr;
  }

  std::memcpy(nptr, ptr, (osize < nsize) ? osize : nsize);
  sinen::GlobalAllocator::get()->deallocate(ptr, osize);
  return nptr;
};

static String toStringTrim(double value) {
  String s(std::format("{}", value));

  auto dot = s.find('.');
  if (dot == String::npos) {
    return s + ".0";
  }

  bool allZero = true;
  for (size_t i = dot + 1; i < s.size(); ++i) {
    if (s[i] != '0') {
      allZero = false;
      break;
    }
  }

  if (allZero) {
    return s.substr(0, dot + 2);
  }

  s.erase(s.find_last_not_of('0') + 1);
  return s;
}
using TablePair = Array<std::pair<String, String>>;
static String convert(StringView name, const TablePair &p, bool isReturn) {
  String s;
  s = name.data();
  s += "{ ";
  if (isReturn) {
    s += "\n";
  }
  for (int i = 0; i < p.size(); i++) {
    auto &v = p[i];
    if (isReturn) {
      s += "\t";
    }
    s += v.first + " = " + v.second;
    if (i < p.size() - 1) {
      s += ", ";
    }
    if (isReturn) {
      s += "\n";
    }
  }
  s += " }";
  return s;
}
class LuaScript final : public IScriptBackend {
public:
  LuaScript() : state(sol::state(sol::default_at_panic, alloc, nullptr)) {}
  ~LuaScript() override { state.collect_gc(); }
  bool initialize() override;
  void finalize() override;

  void runScene(StringView source, StringView chunk) override;

  void update() override;
  void draw() override;

private:
  sol::state state;
};

UniquePtr<IScriptBackend> ScriptBackend::createLua() {
  return makeUnique<LuaScript>();
}
static auto vec3Str(const glm::vec3 &v) {
  TablePair p;
  p.emplace_back("x", toStringTrim(v.x));
  p.emplace_back("y", toStringTrim(v.y));
  p.emplace_back("z", toStringTrim(v.z));
  return convert("sn.Vec3", p, false);
};
static auto vec2Str(const glm::vec2 &v) {
  TablePair p;
  p.emplace_back("x", toStringTrim(v.x));
  p.emplace_back("y", toStringTrim(v.y));

  return convert("sn.Vec2", p, false);
};
static auto textureStr(const Texture &v) {
  TablePair p;
  p.emplace_back("isLoaded", v.texture ? "true" : "false");
  return convert("sn.Texture", p, false);
};
static auto transformStr(const Transform &v) {
  TablePair p;
  p.emplace_back("position", vec3Str(v.position));
  p.emplace_back("rotation", vec3Str(v.rotation));
  p.emplace_back("scale", vec3Str(v.scale));
  return convert("sn.Transform", p, true);
};

static auto colorStr(const Color &v) {
  TablePair p;
  p.emplace_back("r", toStringTrim(v.r));
  p.emplace_back("g", toStringTrim(v.g));
  p.emplace_back("b", toStringTrim(v.b));
  p.emplace_back("a", toStringTrim(v.a));
  return convert("sn.Color", p, false);
};
static auto rectStr(const Rect &rect) {
  return "sn.Rect{ x = " + toStringTrim(rect.x) +
         ", y = " + toStringTrim(rect.y) +
         ", width = " + toStringTrim(rect.width) + ", " + +"}";
};
bool LuaScript::initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                       sol::lib::bit32, sol::lib::io, sol::lib::os,
                       sol::lib::string, sol::lib::debug, sol::lib::table,
                       sol::lib::coroutine, sol::lib::utf8);

  auto lua = state.create_table("sn");
  {
    auto &v = lua;
    state["import"] = [&](StringView path) -> sol::object {
      return state.require_file(path.data(),
                                String(String(path) + ".lua").c_str());
    };
  }
  {
    auto v = lua.new_usertype<glm::vec3>(
        "Vec3", sol::constructors<sol::types<float, float, float>,
                                  sol::types<float>>());

    v["x"] = &glm::vec3::x;
    v["y"] = &glm::vec3::y;
    v["z"] = &glm::vec3::z;
    v["__add"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a + b; };
    v["__sub"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a - b; };
    v["__mul"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a * b; };
    v["__div"] = [](const glm::vec3 &a, const glm::vec3 &b) { return a / b; };
    v["__tostring"] = vec3Str;
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
    auto v = lua.new_usertype<glm::vec2>(
        "Vec2",
        sol::constructors<sol::types<float, float>, sol::types<float>>());
    v["x"] = &glm::vec2::x;
    v["y"] = &glm::vec2::y;
    v["__add"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a + b; };
    v["__sub"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a - b; };
    v["__mul"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a * b; };
    v["__div"] = [](const glm::vec2 &a, const glm::vec2 &b) { return a / b; };
    v["__tostring"] = vec2Str;
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
    auto v = lua.new_usertype<Color>(
        "Color", sol::constructors<sol::types<float, float, float, float>,
                                   sol::types<float, float>, sol::types<float>,
                                   sol::types<>>());
    v["r"] = &Color::r;
    v["g"] = &Color::g;
    v["b"] = &Color::b;
    v["a"] = &Color::a;
    v["__tostring"] = colorStr;
  }
  {
    auto v = lua.new_usertype<Texture>(
        "Texture", sol::constructors<sol::types<int, int>, sol::types<>>());
    v["load"] = &Texture::load;
    v["loadFromPath"] = &Texture::loadFromPath;
    v["fill"] = &Texture::fill;
    v["copy"] = &Texture::copy;
    v["size"] = &Texture::size;
    v["__tostring"] = textureStr;
  }
  {
    auto v = lua.new_usertype<Material>("Material");
    v["appendTexture"] = &Material::appendTexture;
    v["setTexture"] = sol::overload(
        [](Material &m, const Texture &texture) { m.setTexture(texture); },
        [](Material &m, const Texture &texture, size_t index) {
          m.setTexture(texture, index - 1);
        });
    v["setCubemap"] = sol::overload(
        [](Material &m, const Cubemap &cubemap) { m.setCubemap(cubemap); },
        [](Material &m, const Cubemap &cubemap, size_t index) {
          m.setCubemap(cubemap, index);
        });
    v["getTexture"] = &Material::getTexture;
    v["clear"] = &Material::clear;
  }
  {
    auto v = lua.new_usertype<RenderTexture>("RenderTexture");
    v["create"] = &RenderTexture::create;
  }
  {
    auto v = lua.new_usertype<Font>("Font");
    v["load"] = sol::overload(
        [](Font &f, int point_size) { return f.load(point_size); },
        [](Font &f, int point_size, StringView path) {
          return f.load(point_size, path);
        });
    v["loadFromPath"] = &Font::loadFromPath;
    v["resize"] = &Font::resize;
  }
  {
    auto v = lua.new_usertype<Sound>("Sound");
    v["load"] = &Sound::load;
    v["loadFromPath"] = &Sound::loadFromPath;
    v["play"] = &Sound::play;
    v["restart"] = &Sound::restart;
    v["stop"] = &Sound::stop;
    v["setLooping"] = &Sound::setLooping;
    v["setVolume"] = &Sound::setVolume;
    v["setPitch"] = &Sound::setPitch;
    v["setPosition"] = &Sound::setPosition;
    v["setDirection"] = &Sound::setDirection;
    v["isPlaying"] = &Sound::isPlaying;
    v["isLooping"] = &Sound::isLooping;
    v["getVolume"] = &Sound::getVolume;
    v["getPitch"] = &Sound::getPitch;
    v["getPosition"] = &Sound::getPosition;
    v["getDirection"] = &Sound::getDirection;
  }
  {
    auto v = lua.new_usertype<Camera>("Camera");
    v["lookat"] = &Camera::lookat;
    v["perspective"] = &Camera::perspective;
    v["orthographic"] = &Camera::orthographic;
    v["getPosition"] = &Camera::getPosition;
    v["getTarget"] = &Camera::getTarget;
    v["getUp"] = &Camera::getUp;
    v["isAABBInFrustum"] = &Camera::isAABBInFrustum;
  }
  {
    auto v = lua.new_usertype<Camera2D>("Camera2D");
    v["size"] = &Camera2D::size;
    v["half"] = &Camera2D::half;
    v["resize"] = &Camera2D::resize;
    v["windowRatio"] = &Camera2D::windowRatio;
    v["invWindowRatio"] = &Camera2D::invWindowRatio;
  }
  {
    auto v = lua.new_usertype<Model>("Model");
    v["getAABB"] = &Model::getAABB;
    v["load"] = &Model::load;
    v["loadSprite"] = &Model::loadSprite;
    v["loadBox"] = &Model::loadBox;
    v["getBoneUniformData"] = &Model::getBoneUniformData;
    v["play"] = &Model::play;
    v["update"] = &Model::update;
    v["getMaterial"] = &Model::getMaterial;
  }
  {
    auto v = lua.new_usertype<AABB>("AABB");
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
    v["updateWorld"] = &AABB::updateWorld;
  }
  {
    auto v = lua.new_usertype<Timer>("Timer");
    v["start"] = &Timer::start;
    v["stop"] = &Timer::stop;
    v["isStarted"] = &Timer::isStarted;
    v["setTime"] = &Timer::setTime;
    v["check"] = &Timer::check;
  }
  {
    // Collider
    auto v = lua.new_usertype<Collider>("Collider");
    v["getPosition"] = &Collider::getPosition;
    v["getVelocity"] = &Collider::getVelocity;
    v["setLinearVelocity"] = &Collider::setLinearVelocity;
  }
  {
    auto v = lua.new_usertype<UniformData>("UniformData");
    v["add"] = &UniformData::add;
    v["change"] = &UniformData::change;
    v["addCamera"] = &UniformData::addCamera;
    v["addVec3"] = &UniformData::addVec3;
  }
  {

    auto v = lua.new_usertype<Shader>("Shader");
    v["loadVertexShader"] = &Shader::loadVertexShader;
    v["loadFragmentShader"] = &Shader::loadFragmentShader;
    v["compileLoadVertexShader"] = &Shader::compileAndLoadVertexShader;
    v["compileLoadFragmentShader"] = &Shader::compileAndLoadFragmentShader;
  }
  {
    auto v = lua.new_usertype<GraphicsPipeline>("GraphicsPipeline");
    v["setVertexShader"] = &GraphicsPipeline::setVertexShader;
    v["setFragmentShader"] = &GraphicsPipeline::setFragmentShader;
    v["setEnableDepthTest"] = &GraphicsPipeline::setEnableDepthTest;
    v["setEnableInstanced"] = &GraphicsPipeline::setEnableInstanced;
    v["setEnableAnimation"] = &GraphicsPipeline::setEnableAnimation;
    v["setEnableTangent"] = &GraphicsPipeline::setEnableTangent;
    v["build"] = &GraphicsPipeline::build;
  }
  {
    auto v = lua.new_usertype<Draw2D>(
        "Draw2D",
        sol::constructors<sol::types<const Texture &>, sol::types<>>());
    v["position"] = &Draw2D::position;
    v["rotation"] = &Draw2D::rotation;
    v["scale"] = &Draw2D::scale;
    v["material"] = &Draw2D::material;
    v["add"] = &Draw2D::add;
    v["at"] = &Draw2D::at;
    v["clear"] = &Draw2D::clear;
  }
  {
    auto v = lua.new_usertype<Draw3D>(
        "Draw3D",
        sol::constructors<sol::types<const Texture &>, sol::types<>>());

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
    auto v = lua.new_usertype<Rect>(
        "Rect",
        sol::constructors<sol::types<float, float, float, float>,
                          sol::types<glm::vec2, glm::vec2>, sol::types<>>());
    v["x"] = &Rect::x;
    v["y"] = &Rect::y;
    v["width"] = &Rect::width;
    v["height"] = &Rect::height;
  }
  {
    // Transform
    auto v = lua.new_usertype<Transform>("Transform");
    v["__tostring"] = transformStr;
    v["position"] = &Transform::position;
    v["rotation"] = &Transform::rotation;
    v["scale"] = &Transform::scale;
  }
  {
    auto v = lua.new_usertype<Cubemap>("Cubemap");
    v["load"] = &Cubemap::load;
  }
  {
    auto v = lua.new_usertype<Grid<int>>(
        "Grid", sol::constructors<sol::types<int, int>>());
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
    auto v = lua.new_usertype<BFSGrid>(
        "BFSGrid", sol::constructors<sol::types<const Grid<int> &>>());
    v["width"] = &BFSGrid::width;
    v["height"] = &BFSGrid::height;
    v["findPath"] = [](BFSGrid &g, const glm::vec2 &start,
                       const glm::vec2 &end) {
      return g.findPath({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.trace();
      return glm::vec2{t.x + 1, t.y + 1};
    };
    v["traceable"] = &BFSGrid::traceable;
    v["reset"] = &BFSGrid::reset;
  }
  {
    auto v = lua.create_named("Arguments");
    v["getArgc"] = &Arguments::getArgc;
    v["getArgv"] = &Arguments::getArgv;
  }
  {
    auto v = lua.create_named("Random");
    v["getRange"] = &Random::getRange;
    v["getIntRange"] = &Random::getIntRange;
  }
  {
    auto v = lua.create_named("Window");
    v["getName"] = &Window::getName;
    v["size"] = &Window::size;
    v["half"] = Window::half;
    v["resize"] = &Window::resize;
    v["setFullscreen"] = &Window::setFullscreen;
    v["rename"] = &Window::rename;
    v["resized"] = &Window::resized;
  }
  {
    auto v = lua.create_named("Physics");
    v["createBoxCollider"] = &Physics::createBoxCollider;
    v["createSphereCollider"] = &Physics::createSphereCollider;
    v["createCylinderCollider"] = &Physics::createCylinderCollider;
    v["addCollider"] = &Physics::addCollider;
  }
  {
    auto v = lua.create_named("BuiltinPipelines");
    v["get3D"] = &BuiltinPipelines::get3D;
    v["get3DInstanced"] = &BuiltinPipelines::get3DInstanced;
    v["get2D"] = &BuiltinPipelines::get2D;
  }
  {
    auto v = lua.create_named("Graphics");
    v["draw2D"] = &Graphics::draw2D;
    v["draw3D"] = &Graphics::draw3D;
    v["drawRect"] = sol::overload(
        [](const Rect &rect, const Color &color) {
          Graphics::drawRect(rect, color);
        },
        [](const Rect &rect, const Color &color, float angle) {
          Graphics::drawRect(rect, color, angle);
        });
    v["drawImage"] = sol::overload(
        [](const Texture &texture, const Rect &rect) {
          Graphics::drawImage(texture, rect);
        },
        [](const Texture &texture, const Rect &rect, float angle) {
          Graphics::drawImage(texture, rect, angle);
        });
    v["drawText"] = sol::overload(
        [](StringView text, const Font &font, const glm::vec2 &position) {
          Graphics::drawText(text, font, position);
        },
        [](StringView text, const Font &font, const glm::vec2 &position,
           const Color &color) {
          Graphics::drawText(text, font, position, color);
        },
        [](StringView text, const Font &font, const glm::vec2 &position,
           const Color &color, float fontSize) {
          Graphics::drawText(text, font, position, color, fontSize, 0);
        },
        [](StringView text, const Font &font, const glm::vec2 &position,
           const Color &color, float fontSize, float angle) {
          Graphics::drawText(text, font, position, color, fontSize, angle);
        });
    v["drawCubemap"] = &Graphics::drawCubemap;
    v["drawModel"] = &Graphics::drawModel;
    v["drawModelInstanced"] = [](const Model &model, sol::table transformsTable,
                                 const Material &material) {
      Array<Transform> transforms;
      for (auto &item : transformsTable) {
        transforms.push_back(item.second.as<Transform>());
      }
      Graphics::drawModelInstanced(model, transforms, material);
    };
    v["setCamera"] = &Graphics::setCamera;
    v["getCamera"] = &Graphics::getCamera;
    v["setCamera2d"] = &Graphics::setCamera2D;
    v["getCamera2d"] = &Graphics::getCamera2D;
    v["getClearColor"] = &Graphics::getClearColor;
    v["setClearColor"] = &Graphics::setClearColor;
    v["bindPipeline"] = &Graphics::bindPipeline;
    v["setUniformData"] = &Graphics::setUniformData;
    v["setRenderTarget"] = &Graphics::setRenderTarget;
    v["flush"] = &Graphics::flush;
    v["readbackTexture"] = &Graphics::readbackTexture;
  }
  {
    auto v = lua.create_named("Collision");
    v["AABBvsAABB"] = &Collision::aabBvsAabb;
  }
  {
    auto v = lua.create_named("FileSystem");
    v["enumerateDirectory"] = &FileSystem::enumerateDirectory;
  }
  {
    auto v = lua.create_named("Script");
    v["load"] = sol::overload(
        [](StringView filePath) { return Script::load(filePath, "."); },
        [](StringView filePath, StringView baseDirPath) {
          return Script::load(filePath, baseDirPath);
        });
  }
  {
    auto v = lua.create_named("Keyboard");
    v["isPressed"] = &Keyboard::isPressed;
    v["isReleased"] = &Keyboard::isReleased;
    v["isDown"] = &Keyboard::isDown;
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
    v["setRelative"] = &Mouse::setRelative;
    v["isRelative"] = &Mouse::isRelative;
    v["isPressed"] = &Mouse::isPressed;
    v["isReleased"] = &Mouse::isReleased;
    v["isDown"] = &Mouse::isDown;
    v["getPosition"] = &Mouse::getPosition;
    v["getPositionOnScene"] = &Mouse::getPositionOnScene;
    v["setPosition"] = &Mouse::setPosition;
    v["setPositionOnScene"] = &Mouse::setPositionOnScene;
    v["getScrollWheel"] = &Mouse::getScrollWheel;
    v["hideCursor"] = &Mouse::hideCursor;
    v["LEFT"] = (int)Mouse::LEFT;
    v["RIGHT"] = (int)Mouse::RIGHT;
    v["MIDDLE"] = (int)Mouse::MIDDLE;
    v["X1"] = (int)Mouse::X1;
    v["X2"] = (int)Mouse::X2;
  }
  {
    auto v = lua.create_named("Gamepad");
    v["isPressed"] = &GamePad::isPressed;
    v["isReleased"] = &GamePad::isReleased;
    v["isDown"] = &GamePad::isDown;
    v["getLeftStick"] = &GamePad::getLeftStick;
    v["getRightStick"] = &GamePad::getRightStick;
    v["isConnected"] = &GamePad::isConnected;
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
      return Periodic::sin01(period, t);
    };
    v["cos0_1"] = [](float period, float t) {
      return Periodic::cos01(period, t);
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
    v["verbose"] = [](StringView str) { Logger::verbose("%s", str.data()); };
    v["info"] = [](sol::object obj, sol::this_state ts) {
      sol::state_view lua(ts);

      sol::protected_function tostringFn = lua["tostring"];
      sol::protected_function_result r = tostringFn(obj);

      if (!r.valid()) {
        sol::error e = r;
        Logger::error("[tostring error] %s", e.what());
        return;
      }

      auto s = r.get<StringView>();
      Logger::info("%s", s.data());
    };
    v["error"] = [](StringView str) { Logger::error("%s", str.data()); };
    v["warn"] = [](StringView str) { Logger::warn("%s", str.data()); };
    v["critical"] = [](StringView str) { Logger::critical("%s", str.data()); };
  }
#endif
  return true;
}

void LuaScript::finalize() {
#ifndef SINEN_NO_USE_SCRIPT
  state.collect_garbage();
#endif // SINEN_NO_USE_SCRIPT
}

void LuaScript::runScene(StringView source, StringView chunk) {
  state.script(source.data(), chunk.data());
}

void LuaScript::update() { state["Update"](); }

void LuaScript::draw() { state["Draw"](); }

} // namespace sinen