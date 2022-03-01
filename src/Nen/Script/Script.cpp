#include "../Texture/texture_system.hpp"
#include <Nen.hpp>
#include <Nen/DrawObject/draw_object_wrapper.hpp>
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
  impl->state.new_usertype<vector3>("nen_vector3", sol::no_construction(), "x",
                                    &vector3::x, "y", &vector3::y, "z",
                                    &vector3::z);
  impl->state.new_usertype<vector2>("nen_vector2", sol::no_construction(), "x",
                                    &vector2::x, "y", &vector2::y);
  impl->state.new_usertype<color>("nen_color", sol::no_construction(), "r",
                                  &color::r, "g", &color::g, "b", &color::b,
                                  "a", &color::a);
  impl->state.new_usertype<draw2d>(
      "nen_draw2d", sol::no_construction(), "position", &draw2d::position,
      "rotation", &draw2d::rotation, "scale", &draw2d::scale, "texture",
      &draw2d::texture_handle, "draw", &draw2d::draw);
  impl->state.new_usertype<draw3d>(
      "nen_draw3d", sol::no_construction(), "position", &draw3d::position,
      "rotation", &draw3d::rotation, "scale", &draw3d::scale, "texture",
      &draw3d::texture_handle, "draw", &draw3d::draw);
  impl->state.new_usertype<texture>("nen_texture", sol::no_construction(),
                                    "fill_color", &texture::fill_color, "load",
                                    &texture::Load);
  impl->state.new_usertype<font>("nen_font", sol::no_construction(), "load",
                                 &font::LoadFromFile, "render_text",
                                 &font::RenderText);
  impl->state.new_usertype<keyboard_state>(
      "nen_keyboard_state", sol::no_construction(), "is_key_down",
      &keyboard_state::GetKeyValue, "get_key_state",
      &keyboard_state::GetKeyState);
  impl->state.new_usertype<mouse_state>(
      "nen_mouse_state", sol::no_construction(), "get_mouse_state",
      &mouse_state::GetButtonState, "is_button_down",
      &mouse_state::GetButtonValue, "get_position", &mouse_state::GetPosition,
      "get_wheel_state", &mouse_state::GetScrollWheel);
  impl->state.new_enum(
      "key_code", "A", key_code::A, "B", key_code::B, "C", key_code::C, "D",
      key_code::D, "E", key_code::E, "F", key_code::F, "G", key_code::G, "H",
      key_code::H, "I", key_code::I, "J", key_code::J, "K", key_code::K, "L",
      key_code::L, "M", key_code::M, "N", key_code::N, "O", key_code::O, "P",
      key_code::P, "Q", key_code::Q, "R", key_code::R, "S", key_code::S, "T",
      key_code::T, "U", key_code::U, "V", key_code::V, "W", key_code::W, "X",
      key_code::X, "Y", key_code::Y, "Z", key_code::Z, "Key0", key_code::Key0,

      "Key1", key_code::Key1, "Key2", key_code::Key2, "Key3", key_code::Key3,
      "Key4", key_code::Key4, "Key5", key_code::Key5, "Key6", key_code::Key6,
      "Key7", key_code::Key7, "Key8", key_code::Key8, "Key9", key_code::Key9,
      "UP", key_code::UP, "DOWN", key_code::DOWN, "LEFT", key_code::LEFT,
      "RIGHT", key_code::RIGHT, "ESCAPE", key_code::ESCAPE, "SPACE",
      key_code::SPACE, "ENTER", key_code::KP_ENTER, "BACKSPACE",
      key_code::BACKSPACE, "TAB", key_code::TAB, "LSHIFT", key_code::LSHIFT,
      "RSHIFT", key_code::RSHIFT, "LCTRL", key_code::LCTRL, "RCTRL",
      key_code::RCTRL, "ALT", key_code::ALTERASE, "F1", key_code::F1, "F2",
      key_code::F2, "F3", key_code::F3, "F4", key_code::F4, "F5", key_code::F5,
      "F6", key_code::F6, "F7", key_code::F7, "F8", key_code::F8, "F9",
      key_code::F9, "F10", key_code::F10, "F11", key_code::F11, "F12",
      key_code::F12);
  impl->state.new_enum("mouse_code", "LEFT", mouse_code::LEFT, "RIGHT",
                       mouse_code::RIGHT, "MIDDLE", mouse_code::MIDDLE, "X1",
                       mouse_code::X1, "X2", mouse_code::X2);
  impl->state.new_enum("button_state", "NONE", button_state::None, "PRESSED",
                       button_state::Pressed, "RELEASED",
                       button_state::Released, "HELD", button_state::Held);
  /*
  impl->state.new_usertype<music_player>("nen_music", sol::no_construction(),
                                         "load",
                                         &music_player::LoadMusicFromFile,
  "play", &music_player::PlayMusic, "pause", &music_player::
                                         )
  */
  return true;
}

void script_system::DoScript(std::string_view fileName) {
  impl->state.script(asset_reader::LoadAsString(asset_type::Script, fileName));
}

} // namespace nen
