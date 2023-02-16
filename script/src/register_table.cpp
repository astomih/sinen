#include "register_script.hpp"

#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

#include <input/gamepad.hpp>
#include <input/keyboard.hpp>
#include <input/mouse.hpp>

namespace sinen {
void register_table(sol::state &lua) {
  {
    auto v = lua.create_table("window");
    v["name"] = &window::name;
    v["size"] = &window::size;
    v["center"] = window::center;
    v["resize"] = &window::resize;
    v["set_fullscreen"] = &window::set_fullscreen;
    v["rename"] = &window::rename;
    v["resized"] = &window::resized;
  }
  {
    auto v = lua.create_table("renderer");
    v["set_skybox"] = &renderer::set_skybox;
    v["clear_color"] = &renderer::clear_color;
    v["set_clear_color"] = &renderer::set_clear_color;
    v["at_render_texture_user_data"] = &renderer::at_render_texture_user_data;
    v["set_light_lookat"] = &renderer::set_light_look_at;
    v["set_light_ortho"] = &renderer::set_light_ortho;
  }
  {
    auto v = lua.create_table("scene");
    v["load"] = &scene::load;
    v["main_camera"] = &scene::main_camera;
    v["get_actor"] = &scene::get_actor;
    v["size"] = &scene::size;
    v["resize"] = &scene::resize;
    v["center"] = &scene::center;
  }
  {
    auto v = lua.create_table("collision");
    v["aabb_aabb"] = &collision::aabb_aabb;
  }
  {
    auto v = lua.create_table("keyboard");
    v["is_pressed"] = &keyboard::is_pressed;
    v["is_released"] = &keyboard::is_released;
    v["is_down"] = &keyboard::is_down;
    v["A"] = (int)keyboard::A;
    v["B"] = (int)keyboard::B;
    v["C"] = (int)keyboard::C;
    v["D"] = (int)keyboard::D;
    v["E"] = (int)keyboard::E;
    v["F"] = (int)keyboard::F;
    v["G"] = (int)keyboard::G;
    v["H"] = (int)keyboard::H;
    v["I"] = (int)keyboard::I;
    v["J"] = (int)keyboard::J;
    v["K"] = (int)keyboard::K;
    v["L"] = (int)keyboard::L;
    v["M"] = (int)keyboard::M;
    v["N"] = (int)keyboard::N;
    v["O"] = (int)keyboard::O;
    v["P"] = (int)keyboard::P;
    v["Q"] = (int)keyboard::Q;
    v["R"] = (int)keyboard::R;
    v["S"] = (int)keyboard::S;
    v["T"] = (int)keyboard::T;
    v["U"] = (int)keyboard::U;
    v["V"] = (int)keyboard::V;
    v["W"] = (int)keyboard::W;
    v["X"] = (int)keyboard::X;
    v["Y"] = (int)keyboard::Y;
    v["Z"] = (int)keyboard::Z;
    v["key0"] = (int)keyboard::Key0;
    v["key1"] = (int)keyboard::Key1;
    v["key2"] = (int)keyboard::Key2;
    v["key3"] = (int)keyboard::Key3;
    v["key4"] = (int)keyboard::Key4;
    v["key5"] = (int)keyboard::Key5;
    v["key6"] = (int)keyboard::Key6;
    v["key7"] = (int)keyboard::Key7;
    v["key8"] = (int)keyboard::Key8;
    v["key9"] = (int)keyboard::Key9;
    v["F1"] = (int)keyboard::F1;
    v["F2"] = (int)keyboard::F2;
    v["F3"] = (int)keyboard::F3;
    v["F4"] = (int)keyboard::F4;
    v["F5"] = (int)keyboard::F5;
    v["F6"] = (int)keyboard::F6;
    v["F7"] = (int)keyboard::F7;
    v["F8"] = (int)keyboard::F8;
    v["F9"] = (int)keyboard::F9;
    v["F10"] = (int)keyboard::F10;
    v["F11"] = (int)keyboard::F11;
    v["F12"] = (int)keyboard::F12;
    v["UP"] = (int)keyboard::UP;
    v["DOWN"] = (int)keyboard::DOWN;
    v["LEFT"] = (int)keyboard::LEFT;
    v["RIGHT"] = (int)keyboard::RIGHT;
    v["ESCAPE"] = (int)keyboard::ESCAPE;
    v["SPACE"] = (int)keyboard::SPACE;
    v["ENTER"] = (int)keyboard::RETURN;
    v["BACKSPACE"] = (int)keyboard::BACKSPACE;
    v["TAB"] = (int)keyboard::TAB;
    v["LSHIFT"] = (int)keyboard::LSHIFT;
    v["RSHIFT"] = (int)keyboard::RSHIFT;
    v["LCTRL"] = (int)keyboard::LCTRL;
    v["RCTRL"] = (int)keyboard::RCTRL;
    v["ALT"] = (int)keyboard::ALTERASE;
  }
  {
    auto v = lua.create_table("mouse");
    v["is_pressed"] = &mouse::is_pressed;
    v["is_released"] = &mouse::is_released;
    v["is_down"] = &mouse::is_down;
    v["position"] = &mouse::get_position;
    v["position_on_scene"] = &mouse::get_position_on_scene;
    v["set_position"] = &mouse::set_position;
    v["set_position_on_scene"] = &mouse::set_position_on_scene;
    v["scroll_wheel"] = &mouse::get_scroll_wheel;
    v["hide_cursor"] = &mouse::hide_cursor;
    v["LEFT"] = (int)mouse::LEFT;
    v["RIGHT"] = (int)mouse::RIGHT;
    v["MIDDLE"] = (int)mouse::MIDDLE;
    v["X1"] = (int)mouse::X1;
    v["X2"] = (int)mouse::X2;
  }
  {
    auto v = lua.create_table("gamepad");
    v["is_pressed"] = &gamepad::is_pressed;
    v["is_released"] = &gamepad::is_released;
    v["is_down"] = &gamepad::is_down;
    v["left_stick"] = &gamepad::get_left_stick;
    v["right_stick"] = &gamepad::get_right_stick;
    v["is_connected"] = &gamepad::is_connected;
    v["INVALID"] = (int)gamepad::INVALID;
    v["A"] = (int)gamepad::A;
    v["B"] = (int)gamepad::B;
    v["X"] = (int)gamepad::X;
    v["Y"] = (int)gamepad::Y;
    v["BACK"] = (int)gamepad::BACK;
    v["GUIDE"] = (int)gamepad::GUIDE;
    v["START"] = (int)gamepad::START;
    v["LEFTSTICK"] = (int)gamepad::LEFTSTICK;
    v["RIGHTSTICK"] = (int)gamepad::RIGHTSTICK;
    v["LEFTSHOULDER"] = (int)gamepad::LEFTSHOULDER;
    v["RIGHTSHOULDER"] = (int)gamepad::RIGHTSHOULDER;
    v["DPAD_UP"] = (int)gamepad::DPAD_UP;
    v["DPAD_DOWN"] = (int)gamepad::DPAD_DOWN;
    v["DPAD_LEFT"] = (int)gamepad::DPAD_LEFT;
    v["DPAD_RIGHT"] = (int)gamepad::DPAD_RIGHT;
    v["MISC1"] = (int)gamepad::MISC1;
    v["PADDLE1"] = (int)gamepad::PADDLE1;
    v["PADDLE2"] = (int)gamepad::PADDLE2;
    v["PADDLE3"] = (int)gamepad::PADDLE3;
    v["PADDLE4"] = (int)gamepad::PADDLE4;
    v["TOUCHPAD"] = (int)gamepad::TOUCHPAD;
  }
}
} // namespace sinen
