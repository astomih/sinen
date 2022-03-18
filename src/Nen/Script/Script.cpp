#include "../Texture/texture_system.hpp"
#include <Audio/MusicSystem.hpp>
#include <Audio/SoundEvent.hpp>
#include <Audio/SoundSystem.hpp>
#include <DrawObject/draw_object_wrapper.hpp>
#include <Font/Font.hpp>
#include <IO/AssetReader.hpp>
#include <Input/InputSystem.hpp>
#include <Render/Renderer.hpp>
#include <Script/Script.hpp>
#include <Window/Window.hpp>
#include <camera/camera.hpp>
#include <functional>
#include <instancing/instancing_wrapper.hpp>
#include <manager/manager.hpp>
#include <model/model.hpp>
#include <sol/sol.hpp>

namespace nen {
script_system::script_system(manager &_manager)
    : m_manager(_manager), impl(std::make_unique<implement>()) {}
script_system::~script_system() = default;
class script_system::implement {
public:
  sol::state state;
};
void *script_system::get_state() { return (void *)&impl->state; }
bool script_system::initialize() {
#ifndef NEN_NO_USE_SCRIPT
  impl->state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math,
                             sol::lib::bit32);
  impl->state["require"] = [&](const std::string &str) -> sol::object {
    return impl->state.require_script(
        str, nen::data_io::LoadAsString(nen::asset_type::Script, str + ".lua"));
  };
  impl->state["texture"] = [&]() -> texture { return texture(); };
  impl->state["font"] = [&]() -> font { return font(); };
  impl->state["DEFAULT_FONT"] = "mplus/mplus-1p-medium.ttf";
  impl->state["draw2d"] = [&]() -> draw2d { return draw2d(); };
  impl->state["draw2d"] = [&](texture t) -> draw2d { return draw2d(t); };
  impl->state["draw3d"] = [&]() -> draw3d { return draw3d(); };
  impl->state["draw3d"] = [&](texture t) -> draw3d { return draw3d(t); };
  impl->state["draw2d_instanced"] = [&](texture t) -> draw2d_instancing {
    return draw2d_instancing(t);
  };
  impl->state["draw3d_instanced"] = [&](texture t) -> draw3d_instancing {
    return draw3d_instancing(t);
  };
  impl->state["vector3"] = [&](float x, float y, float z) -> vector3 {
    return vector3(x, y, z);
  };
  impl->state["vector2"] = [&](float x, float y) -> vector2 {
    return vector2(x, y);
  };
  impl->state["quaternion"] = [&](sol::this_state s) -> quaternion {
    return quaternion();
  };
  impl->state["color"] = [&](float r, float g, float b, float a) -> color {
    return color(r, g, b, a);
  };
  impl->state["model"] = [&]() -> model { return model(); };
  impl->state["music"] = [&]() -> music { return music(); };
  impl->state["sound"] = [&]() -> sound { return sound(); };
  impl->state["aabb"] = [&]() -> aabb { return aabb(); };
  {
    auto v = impl->state.new_usertype<vector3>("nen_vector3",
                                               sol::no_construction());
    v["x"] = &vector3::x;
    v["y"] = &vector3::y;
    v["z"] = &vector3::z;
    v["add"] = &vector3::add;
    v["sub"] = &vector3::sub;
    v["mul"] = &vector3::mul;
    v["div"] = &vector3::div;
    v["copy"] = &vector3::copy;
  }
  {
    auto v = impl->state.new_usertype<vector2>("nen_vector2",
                                               sol::no_construction());
    v["x"] = &vector2::x;
    v["y"] = &vector2::y;
    v["add"] = &vector2::add;
    v["sub"] = &vector2::sub;
    v["mul"] = &vector2::mul;
    v["div"] = &vector2::div;
  }
  {
    auto v =
        impl->state.new_usertype<color>("nen_color", sol::no_construction());
    v["r"] = &color::r;
    v["g"] = &color::g;
    v["b"] = &color::b;
    v["a"] = &color::a;
  }
  {
    auto v =
        impl->state.new_usertype<draw2d>("nen_draw2d", sol::no_construction());
    v["draw"] = &draw2d::draw;
    v["position"] = &draw2d::position;
    v["rotation"] = &draw2d::rotation;
    v["scale"] = &draw2d::scale;
    v["texture"] = &draw2d::texture_handle;
    v["vertex_name"] = &draw2d::vertex_name;
  }
  {
    auto v =
        impl->state.new_usertype<draw3d>("nen_draw3d", sol::no_construction());
    v["position"] = &draw3d::position;
    v["rotation"] = &draw3d::rotation;
    v["scale"] = &draw3d::scale;
    v["texture"] = &draw3d::texture_handle;
    v["draw"] = &draw3d::draw;
    v["vertex_name"] = &draw3d::vertex_name;
  }
  {
    auto v = impl->state.new_usertype<draw2d_instancing>(
        "nen_draw2d_instanced", sol::no_construction());
    v["draw"] = &draw2d_instancing::draw;
    v["add"] = &draw2d_instancing::add;
    v["texture"] = &draw2d_instancing::texture_handle;
  }
  {
    auto v = impl->state.new_usertype<draw3d_instancing>(
        "nen_draw3d_instanced", sol::no_construction());
    v["draw"] = &draw3d_instancing::draw;
    v["add"] = &draw3d_instancing::add;
    v["texture"] = &draw3d_instancing::texture_handle;
  }
  {
    auto v = impl->state.new_usertype<texture>("nen_texture",
                                               sol::no_construction());
    v["fill_color"] = &texture::fill_color;
    v["load"] = &texture::Load;
    v["size"] = &texture::size;
  }
  {
    auto v = impl->state.new_usertype<font>("nen_font", sol::no_construction());
    v["load"] = &font::LoadFromFile;
    v["render_text"] = &font::RenderText;
  }
  {
    auto v = impl->state.new_usertype<keyboard_state>("nen_keyboard_state",
                                                      sol::no_construction());
    v["is_key_down"] = &keyboard_state::GetKeyValue;
    v["key_state"] = &keyboard_state::GetKeyState;
  }
  {
    auto v = impl->state.new_usertype<mouse_state>("nen_mouse_state",
                                                   sol::no_construction());
    v["button_state"] = &mouse_state::button_state;
    v["is_button_down"] = &mouse_state::is_button_down;
    v["position"] = &mouse_state::GetPosition;
    v["wheel_state"] = &mouse_state::GetScrollWheel;
  }
  {
    auto v =
        impl->state.new_usertype<music>("nen_music", sol::no_construction());
    v["load"] = &music::LoadMusicFromFile;
    v["play"] = &music::PlayMusic;
    v["set_volume"] = &music::set_volume;
  }
  {
    auto v =
        impl->state.new_usertype<sound>("nen_sound", sol::no_construction());
    v["load"] = &sound::load;
    v["play"] = &sound::play;
    v["set_volume"] = &sound::SetVolume;
    v["set_pitch"] = &sound::SetPitch;
    v["set_listener"] = &sound::set_listener;
    v["set_position"] = &sound::SetPosition;
  }
  {
    auto v =
        impl->state.new_usertype<camera>("nen_camera", sol::no_construction());
    v["position"] = &camera::position;
    v["aspect"] = &camera::aspect;
    v["far"] = &camera::far;
    v["near"] = &camera::near;
    v["fov"] = &camera::fov;
    v["target"] = &camera::target;
    v["up"] = &camera::up;
    v["update"] = &camera::update;
  }
  impl->state["change_scene"] = [&](const std::string &str) {
    get_manager().change_scene(str);
  };
  {
    auto v =
        impl->state.new_usertype<model>("nen_model", sol::no_construction());
    v["aabb"] = &model::m_aabb;
    v["load"] = &model::load;
  }
  {
    auto v = impl->state.new_usertype<aabb>("nen_aabb", sol::no_construction());
    v["min"] = &aabb::min;
    v["max"] = &aabb::max;
    v["intersects_aabb"] = &aabb::intersects_aabb;
  }
  {
    auto &v = impl->state;
    v["keyA"] = key_code::A;
    v["keyB"] = key_code::B;
    v["keyC"] = key_code::C;
    v["keyD"] = key_code::D;
    v["keyE"] = key_code::E;
    v["keyF"] = key_code::F;
    v["keyG"] = key_code::G;
    v["keyH"] = key_code::H;
    v["keyI"] = key_code::I;
    v["keyJ"] = key_code::J;
    v["keyK"] = key_code::K;
    v["keyL"] = key_code::L;
    v["keyM"] = key_code::M;
    v["keyN"] = key_code::N;
    v["keyO"] = key_code::O;
    v["keyP"] = key_code::P;
    v["keyQ"] = key_code::Q;
    v["keyR"] = key_code::R;
    v["keyS"] = key_code::S;
    v["keyT"] = key_code::T;
    v["keyU"] = key_code::U;
    v["keyV"] = key_code::V;
    v["keyW"] = key_code::W;
    v["keyX"] = key_code::X;
    v["keyY"] = key_code::Y;
    v["keyZ"] = key_code::Z;
    v["key0"] = key_code::Key0;
    v["key1"] = key_code::Key1;
    v["key2"] = key_code::Key2;
    v["key3"] = key_code::Key3;
    v["key4"] = key_code::Key4;
    v["key5"] = key_code::Key5;
    v["key6"] = key_code::Key6;
    v["key7"] = key_code::Key7;
    v["key8"] = key_code::Key8;
    v["key9"] = key_code::Key9;
    v["keyF1"] = key_code::F1;
    v["keyF2"] = key_code::F2;
    v["keyF3"] = key_code::F3;
    v["keyF4"] = key_code::F4;
    v["keyF5"] = key_code::F5;
    v["keyF6"] = key_code::F6;
    v["keyF7"] = key_code::F7;
    v["keyF8"] = key_code::F8;
    v["keyF9"] = key_code::F9;
    v["keyF10"] = key_code::F10;
    v["keyF11"] = key_code::F11;
    v["keyF12"] = key_code::F12;
    v["keyUP"] = key_code::UP;
    v["keyDOWN"] = key_code::DOWN;
    v["keyLEFT"] = key_code::LEFT;
    v["keyRIGHT"] = key_code::RIGHT;
    v["keyESCAPE"] = key_code::ESCAPE;
    v["keySPACE"] = key_code::SPACE;
    v["keyENTER"] = key_code::KP_ENTER;
    v["keyBACKSPACE"] = key_code::BACKSPACE;
    v["keyTAB"] = key_code::TAB;
    v["keyLSHIFT"] = key_code::LSHIFT;
    v["keyRSHIFT"] = key_code::RSHIFT;
    v["keyLCTRL"] = key_code::LCTRL;
    v["keyRCTRL"] = key_code::RCTRL;
    v["keyALT"] = key_code::ALTERASE;
    v["keyLCTRL"] = key_code::LCTRL;
    v["keyRCTRL"] = key_code::RCTRL;
  }
  {
    auto &v = impl->state;
    v["mouseLEFT"] = mouse_code::LEFT;
    v["mouseRIGHT"] = mouse_code::RIGHT;
    v["mouseMIDDLE"] = mouse_code::MIDDLE;
    v["mouseX1"] = mouse_code::X1;
    v["mouseX2"] = mouse_code::X2;
  }
  {
    auto &v = impl->state;
    v["buttonNONE"] = button_state::None;
    v["buttonPRESSED"] = button_state::Pressed;
    v["buttonRELEASED"] = button_state::Released;
    v["buttonHELD"] = button_state::Held;
  }
#endif
  return true;
}

void script_system::DoScript(std::string_view fileName) {
  impl->state.script(data_io::LoadAsString(asset_type::Script, fileName));
}

} // namespace nen
