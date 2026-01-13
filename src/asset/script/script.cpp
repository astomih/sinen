// internal
#include "script.hpp"

#include <asset/asset.hpp>
#include <asset/shader/builtin_shader.hpp>
#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>
#include <core/event/event.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <math/graph/bfs_grid.hpp>
#include <math/math.hpp>
#include <math/periodic.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/gamepad.hpp>
#include <platform/input/key_input.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/input/mouse.hpp>
#include <platform/io/arguments.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

#include <sol/raii.hpp>
#include <sol/types.hpp>

#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include <sol/sol.hpp>

#include <format>

namespace sinen {
void bindImGui(sol::table &lua);
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
sol::state state;
static auto vec3Str(const Vec3 &v) {
  TablePair p;
  p.emplace_back("x", toStringTrim(v.x));
  p.emplace_back("y", toStringTrim(v.y));
  p.emplace_back("z", toStringTrim(v.z));
  return convert("sn.Vec3", p, false);
};
static auto vec2Str(const Vec2 &v) {
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

bool Script::initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  state = sol::state(sol::default_at_panic, alloc, nullptr);
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

    auto v = lua.new_usertype<Buffer>("Buffer", sol::no_construction());
    v["new"] = [](sol::table t) {
      Array<void *> arr;
      Array<size_t> arrS;
      for (auto &i : t) {
        auto &value = i.second;
        if (value.get_type() == sol::type::userdata) {
          if (value.is<Vec3>()) {
            auto v = value.as<Vec3>();
            auto s = sizeof(Vec3);
            auto *p = GlobalAllocator::get()->allocate(s);
            memcpy(p, &v, s);
            arr.push_back(p);
            arrS.push_back(s);
          } else if (value.is<Vec2>()) {
            auto v = value.as<Vec2>();
            auto s = sizeof(Vec2);
            auto *p = GlobalAllocator::get()->allocate(s);
            memcpy(p, &v, s);
            arr.push_back(p);
            arrS.push_back(s);
          } else if (value.is<Camera>()) {

            auto v = value.as<Camera>();
            auto s = sizeof(Mat4) * 2;
            auto *p = GlobalAllocator::get()->allocate(s);
            auto view = (v.getView());
            auto proj = (v.getProjection());
            memcpy(p, &view, sizeof(Mat4));
            memcpy((void *)(reinterpret_cast<std::byte *>(p) + sizeof(Mat4)),
                   &proj, sizeof(Mat4));
            arr.push_back(p);
            arrS.push_back(s);
          }
        } else if (value.is<double>()) {
          auto v = value.as<float>();
          auto s = sizeof(float);
          auto *p = GlobalAllocator::get()->allocate(s);
          memcpy(p, &v, s);
          arr.push_back(p);
          arrS.push_back(s);
        }
      }
      size_t size = 0;
      for (auto &i : arrS) {
        size += i;
      }
      auto *ptr = GlobalAllocator::get()->allocate(size);
      size_t offset = 0;
      for (int i = 0; i < arr.size(); i++) {
        memcpy(reinterpret_cast<void *>(reinterpret_cast<std::byte *>(ptr) +
                                        offset),
               arr[i], arrS[i]);
        GlobalAllocator::get()->deallocate(arr[i], arrS[i]);
        arr[i] = nullptr;
        offset += arrS[i];
      }
      arr.clear();
      arrS.clear();

      return Buffer(
          BufferType::Binary,
          Ptr<void>(ptr, DeleterWithSize<void>(GlobalAllocator::get(), size)),
          size);
    };
  }
  {
    auto v = lua.new_usertype<Vec3>(
        "Vec3", sol::constructors<sol::types<float, float, float>,
                                  sol::types<float>>());

    v["x"] = &Vec3::x;
    v["y"] = &Vec3::y;
    v["z"] = &Vec3::z;
    v["__add"] = [](const Vec3 &a, const Vec3 &b) { return a + b; };
    v["__sub"] = [](const Vec3 &a, const Vec3 &b) { return a - b; };
    v["__mul"] = [](const Vec3 &a, const Vec3 &b) { return a * b; };
    v["__div"] = [](const Vec3 &a, const Vec3 &b) { return a / b; };
    v["__tostring"] = vec3Str;
    v["copy"] = [](const Vec3 &a) { return a; };
    v["length"] = [](const Vec3 &a) { return a.length(); };
    v["forward"] = [](const Vec3 v, const Vec3 rotation) -> Vec3 {
      // rotation to mat
      auto m = Mat4::fromQuat(Quat::fromEuler(rotation));
      // forward
      Vec4 forward = m * Vec4(v.x, v.y, v.z, 1.0f);
      forward.x = forward.x / forward.w;
      forward.y = forward.y / forward.w;
      forward.z = forward.z / forward.w;

      return Vec3(forward.x, forward.y, forward.z);
    };
    v["normalize"] = [](const Vec3 &v) { return Vec3::normalize(v); };
    v["dot"] = [](const Vec3 &a, const Vec3 &b) { return Vec3::dot(a, b); };
    v["cross"] = [](const Vec3 &a, const Vec3 &b) { return Vec3::cross(a, b); };
    v["lerp"] = [](const Vec3 &a, const Vec3 &b, float t) {
      return Vec3::lerp(a, b, t);
    };
    v["reflect"] = [](const Vec3 &v, const Vec3 &n) {
      return Vec3::reflect(v, n);
    };
  }
  {
    auto v = lua.new_usertype<Vec2>(
        "Vec2",
        sol::constructors<sol::types<float, float>, sol::types<float>>());
    v["x"] = &Vec2::x;
    v["y"] = &Vec2::y;
    v["__add"] = [](const Vec2 &a, const Vec2 &b) { return a + b; };
    v["__sub"] = [](const Vec2 &a, const Vec2 &b) { return a - b; };
    v["__mul"] = [](const Vec2 &a, const Vec2 &b) { return a * b; };
    v["__div"] = [](const Vec2 &a, const Vec2 &b) { return a / b; };
    v["__tostring"] = vec2Str;
    v["copy"] = [](const Vec2 &a) { return a; };
    v["length"] = [](const Vec2 &a) { return a.length(); };
    v["normalize"] = [](const Vec2 &v) { return Vec2::normalize(v); };
    v["dot"] = [](const Vec2 &a, const Vec2 &b) { return Vec2::dot(a, b); };
    v["lerp"] = [](const Vec2 &a, const Vec2 &b, float t) {
      return Vec2::lerp(a, b, t);
    };
    v["reflect"] = [](const Vec2 &v, const Vec2 &n) {
      return Vec2::reflect(v, n);
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
    v["load"] = sol::overload(
        [](Texture &texture, StringView s) { texture.load(s); },
        [](Texture &texture, const Buffer &buffer) { texture.load(buffer); });
    v["loadCubemap"] = [](Texture &texture, StringView s) {
      texture.loadCubemap(s);
    };
    v["fill"] = &Texture::fill;
    v["copy"] = &Texture::copy;
    v["size"] = &Texture::size;
    v["__tostring"] = textureStr;
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
        },
        [](Font &f, int point_size, const Buffer &buffer) {
          return f.load(point_size, buffer);
        });
    v["resize"] = &Font::resize;
  }
  {
    auto v = lua.new_usertype<Sound>("Sound");
    v["load"] = sol::overload(
        [](Sound &sound, StringView s) { sound.load(s); },
        [](Sound &sound, const Buffer &buffer) { sound.load(buffer); });
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
    auto v = lua.create_named("TextureKey");
    v["BaseColor"] = TextureKey::BaseColor;
    v["Normal"] = TextureKey::Normal;
    v["DiffuseRoughness"] = TextureKey::DiffuseRoughness;
    v["Metalness"] = TextureKey::Metalness;
    v["Emissive"] = TextureKey::Emissive;
    v["LightMap"] = TextureKey::LightMap;
  }
  {
    auto v = lua.new_usertype<Model>("Model");
    v["getAABB"] = &Model::getAABB;
    v["load"] = sol::overload(
        [](Model &model, StringView s) { model.load(s); },
        [](Model &model, const Buffer &buffer) { model.load(buffer); });
    v["loadSprite"] = &Model::loadSprite;
    v["loadBox"] = &Model::loadBox;
    v["getBoneUniformBuffer"] = &Model::getBoneUniformBuffer;
    v["play"] = &Model::play;
    v["update"] = &Model::update;
    v["hasTexture"] = &Model::hasTexture;
    v["getTexture"] = &Model::getTexture;
    v["setTexture"] = &Model::setTexture;
  }
  {
    auto v = lua.new_usertype<AABB>("AABB");
    v["min"] = &AABB::min;
    v["max"] = &AABB::max;
    v["updateWorld"] = &AABB::updateWorld;
    v["intersectsAABB"] = &AABB::intersectsAABB;
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
    auto v = lua.new_usertype<Shader>("Shader");
    v["load"] = &Shader::load;
    v["compileAndLoad"] = &Shader::compileAndLoad;
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
    v["model"] = &Draw3D::model;
    v["add"] = &Draw3D::add;
    v["at"] = &Draw3D::at;
    v["clear"] = &Draw3D::clear;
  }
  {
    // Rect
    auto v = lua.new_usertype<Rect>(
        "Rect", sol::constructors<sol::types<float, float, float, float>,
                                  sol::types<Vec2, Vec2>, sol::types<>>());
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
    auto v = lua.new_usertype<Grid>(
        "Grid", sol::constructors<sol::types<float, float>>());
    v["at"] = [](Grid &g, int x, int y) { return g.at(x - 1, y - 1); };
    v["set"] = [](Grid &g, int x, int y, float v) {
      return g.at(x - 1, y - 1) = v;
    };
    v["width"] = &Grid::width;
    v["height"] = &Grid::height;
    v["size"] = &Grid::size;
    v["clear"] = &Grid::clear;
    v["resize"] = &Grid::resize;
    v["fill"] = &Grid::fill;
    v["fillRect"] = [](Grid &grid, const Rect &rect, int value) {
      grid.fillRect({rect.x + 1, rect.y + 1, rect.width, rect.height}, value);
    };
    v["setRow"] = [](Grid &grid, int index, float value) {
      grid.setRow(index + 1, value);
    };
    v["setColumn"] = [](Grid &grid, int index, float value) {
      grid.setColumn(index + 1, value);
    };
  }
  {
    auto v = lua.new_usertype<BFSGrid>(
        "BFSGrid", sol::constructors<sol::types<const Grid &>>());
    v["width"] = &BFSGrid::width;
    v["height"] = &BFSGrid::height;
    v["findPath"] = [](BFSGrid &g, const Vec2 &start, const Vec2 &end) {
      return g.findPath({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.trace();
      return Vec2{t.x + 1, t.y + 1};
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
    v["getName"] = &Window::name;
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
    auto v = lua.create_named("ShaderStage");
    v["Vertex"] = static_cast<UInt32>(ShaderStage::Vertex);
    v["Fragment"] = static_cast<UInt32>(ShaderStage::Fragment);
    v["Compute"] = static_cast<UInt32>(ShaderStage::Compute);
  }
  {
    auto v = lua.create_named("BuiltinShader");
    v["getDefaultVS"] = &BuiltinShader::getDefaultVS;
    v["getDefaultFS"] = &BuiltinShader::getDefaultFS;
    v["getDefaultInstancedVS"] = &BuiltinShader::getDefaultInstancedVS;
    v["getCubemapVS"] = &BuiltinShader::getCubemapVS;
    v["getCubemapFS"] = &BuiltinShader::getCubemapFS;
  }
  {
    auto v = lua.create_named("BuiltinPipeline");
    v["getDefault3D"] = &BuiltinPipeline::getDefault3D;
    v["getInstanced3D"] = &BuiltinPipeline::getInstanced3D;
    v["getDefault2D"] = &BuiltinPipeline::getDefault2D;
    v["getCubemap"] = &BuiltinPipeline::getCubemap;
  }
  {
    auto v = lua.create_named("Event");
    v["quit"] = &Event::quit;
  }
  {
    auto v = lua.create_named("Graphics");
    v["draw2D"] = &Graphics::drawBase2D;
    v["draw3D"] = &Graphics::drawBase3D;
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
        [](StringView text, const Font &font, const Vec2 &position) {
          Graphics::drawText(text, font, position);
        },
        [](StringView text, const Font &font, const Vec2 &position,
           const Color &color) {
          Graphics::drawText(text, font, position, color);
        },
        [](StringView text, const Font &font, const Vec2 &position,
           const Color &color, float fontSize) {
          Graphics::drawText(text, font, position, color, fontSize, 0);
        },
        [](StringView text, const Font &font, const Vec2 &position,
           const Color &color, float fontSize, float angle) {
          Graphics::drawText(text, font, position, color, fontSize, angle);
        });
    v["drawCubemap"] = &Graphics::drawCubemap;
    v["drawModel"] = &Graphics::drawModel;
    v["drawModelInstanced"] = [](const Model &model,
                                 sol::table transformsTable) {
      Array<Transform> transforms;
      for (auto &item : transformsTable) {
        transforms.push_back(item.second.as<Transform>());
      }
      Graphics::drawModelInstanced(model, transforms);
    };
    v["setCamera"] = &Graphics::setCamera;
    v["getCamera"] = &Graphics::getCamera;
    v["setCamera2d"] = &Graphics::setCamera2D;
    v["getCamera2d"] = &Graphics::getCamera2D;
    v["getClearColor"] = &Graphics::getClearColor;
    v["setClearColor"] = &Graphics::setClearColor;
    v["setGraphicsPipeline"] = &Graphics::setGraphicsPipeline;
    v["resetGraphicsPipeline"] = &Graphics::resetGraphicsPipeline;
    v["setTexture"] = &Graphics::setTexture;
    v["resetTexture"] = &Graphics::resetTexture;
    v["resetAllTexture"] = &Graphics::resetAllTexture;
    v["setUniformBuffer"] = &Graphics::setUniformBuffer;
    v["setRenderTarget"] = &Graphics::setRenderTarget;
    v["flush"] = &Graphics::flush;
    v["readbackTexture"] = &Graphics::readbackTexture;
  }
  {
    auto v = lua.create_named("Filesystem");
    v["enumerateDirectory"] = &Filesystem::enumerateDirectory;
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
      return Periodic::sineWave(period, t);
    };
    v["cos0_1"] = [](float period, float t) {
      return Periodic::cosWave(period, t);
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
  bindImGui(lua);
#endif
  return true;
}
void Script::shutdown() {
#ifndef SINEN_NO_USE_SCRIPT
  state.collect_garbage();
#endif // SINEN_NO_USE_SCRIPT
}

static const char *nothingSceneLua = R"(
local font = sn.Font.new()
font:load(32)
function Update()
end

function Draw()
    sn.Graphics.drawText("NO DATA", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32, 0.0)
end
)";
sol::function setup, update, draw;
void Script::runScene() {

  String source;
  source = AssetIO::openAsString(String(sceneName) + ".lua");
  if (source.empty()) {
    source = nothingSceneLua;
  }
  state.script(
      source.data(),
      StringView("@" + AssetIO::getFilePath(sceneName) + ".lua").data());
  setup = state["Setup"];
  update = state["Update"];
  draw = state["Draw"];
  if (setup.valid()) {
    setup();
  }
}

void Script::updateScene() {
  if (update.valid()) {
    update();
  }
}

void Script::drawScene() {
  if (draw.valid()) {
    draw();
  }
}

} // namespace sinen