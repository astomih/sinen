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
#include <functional>
#include <sol/sol.hpp>


namespace nen {
draw2d::draw2d()
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(10.f, 10.f)) {}
draw2d::draw2d(texture texture_handle)
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(10.f, 10.f)),
      texture_handle(texture_handle) {}
draw3d::draw3d()
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(10.f, 10.f, 10.f)) {}
draw3d::draw3d(texture texture_handle)
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(10.f, 10.f, 10.f)), texture_handle(texture_handle) {}
void draw2d::draw() {
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::NegUnitZ, rotation);
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  matrix4 ts = matrix4::Identity;
  ts.mat[0][0] = static_cast<float>(texture(texture_handle).GetWidth());
  ts.mat[1][1] = static_cast<float>(texture(texture_handle).GetHeight());
  obj->param.world = ts * s * r * t;
  obj->texture_handle = texture_handle.handle;
  matrix4 viewproj = matrix4::Identity;

  auto windowsize = get_window().Size();
  viewproj.mat[0][0] = 1.f / windowsize.x;
  viewproj.mat[1][1] = 1.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::Identity;
  obj->vertexIndex = "SPRITE";
  get_renderer().draw2d(obj);
}
void draw3d::draw() {
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  t.mat[3][2] = position.z;
  quaternion q(vector3::NegUnitZ, rotation.z);
  q = quaternion::Concatenate(q, quaternion(vector3::UnitY, rotation.y));
  q = quaternion::Concatenate(q, quaternion(vector3::UnitX, rotation.x));
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  s.mat[2][2] = scale.z;
  obj->param.world = s * r * t;
  obj->texture_handle = texture_handle.handle;
  obj->param.proj = get_renderer().GetProjectionMatrix();
  obj->param.view = get_renderer().GetViewMatrix();
  obj->vertexIndex = "SPRITE";
  get_renderer().draw3d(obj);
}
script_system::script_system(manager &_manager)
    : m_manager(_manager), impl(std::make_unique<implement>()) {}
script_system::~script_system() = default;
class script_system::implement {
public:
  sol::state state;
};
void *script_system::get_state() { return (void *)&impl->state; }
bool script_system::initialize() {
  impl->state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
  impl->state["require"] = [&](const std::string &str) -> sol::object {
    return impl->state.require_script(
        str,
        nen::asset_reader::LoadAsString(nen::asset_type::Script, str) + ".lua");
  };
  impl->state["texture"] = [&]() -> texture { return texture(); };
  impl->state["font"] = [&]() -> font { return font(); };
  impl->state["DEFAULT_FONT"] = "mplus/mplus-1p-medium.ttf";
  impl->state["draw2d"] = [&]() -> draw2d { return draw2d(); };
  impl->state["draw2d"] = [&](texture t) -> draw2d { return draw2d(t); };
  impl->state["draw3d"] = [&]() -> draw3d { return draw3d(); };
  impl->state["draw3d"] = [&](texture t) -> draw3d { return draw3d(t); };
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
  impl->state["music"] = [&]() -> music { return music(); };
  impl->state["sound"] = [&]() -> sound { return sound(); };
  {
    auto v = impl->state.new_usertype<vector3>("nen_vector3",
                                               sol::no_construction());
    v["x"] = &vector3::x;
    v["y"] = &vector3::y;
    v["z"] = &vector3::z;
  }
  {
    auto v = impl->state.new_usertype<vector2>("nen_vector2",
                                               sol::no_construction());
    v["x"] = &vector2::x;
    v["y"] = &vector2::y;
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
  }
  {
    auto v =
        impl->state.new_usertype<draw3d>("nen_draw3d", sol::no_construction());
    v["position"] = &draw3d::position;
    v["rotation"] = &draw3d::rotation;
    v["scale"] = &draw3d::scale;
    v["texture"] = &draw3d::texture_handle;
    v["draw"] = &draw3d::draw;
  }
  {
    auto v = impl->state.new_usertype<texture>("nen_texture",
                                               sol::no_construction());
    v["fill_color"] = &texture::fill_color;
    v["load"] = &texture::Load;
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
    v["get_key_state"] = &keyboard_state::GetKeyState;
  }
  {
    auto v = impl->state.new_usertype<mouse_state>("nen_mouse_state",
                                                   sol::no_construction());
    v["get_mouse_state"] = &mouse_state::GetButtonState;
    v["is_button_down"] = &mouse_state::GetButtonValue;
    v["get_position"] = &mouse_state::GetPosition;
    v["get_wheel_state"] = &mouse_state::GetScrollWheel;
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
  return true;
}

void script_system::DoScript(std::string_view fileName) {
  impl->state.script(asset_reader::LoadAsString(asset_type::Script, fileName));
}

} // namespace nen
