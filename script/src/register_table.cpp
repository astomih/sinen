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
    v["state"] = []() { return static_cast<int>(window::state()); };
    v["size"] = &window::size;
    v["center"] = window::center;
    v["set_size"] = &window::set_size;
    v["set_fullscreen"] = &window::set_fullscreen;
    v["set_name"] = &window::set_name;
  }
  {
    auto v = lua.create_table("renderer");
    v["set_skybox"] = &renderer::set_skybox;
    v["skybox"] = &renderer::skybox;
    v["clear_color"] = &renderer::clear_color;
    v["set_clear_color"] = &renderer::set_clear_color;
    v["get_graphics_api"] = &renderer::get_graphics_api;
    v["at_render_texture_user_data"] = &renderer::at_render_texture_user_data;
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
    v["A"] = (int)keyboard::code::A;
    v["B"] = (int)keyboard::code::B;
    v["C"] = (int)keyboard::code::C;
    v["D"] = (int)keyboard::code::D;
    v["E"] = (int)keyboard::code::E;
    v["F"] = (int)keyboard::code::F;
    v["G"] = (int)keyboard::code::G;
    v["H"] = (int)keyboard::code::H;
    v["I"] = (int)keyboard::code::I;
    v["J"] = (int)keyboard::code::J;
    v["K"] = (int)keyboard::code::K;
    v["L"] = (int)keyboard::code::L;
    v["M"] = (int)keyboard::code::M;
    v["N"] = (int)keyboard::code::N;
    v["O"] = (int)keyboard::code::O;
    v["P"] = (int)keyboard::code::P;
    v["Q"] = (int)keyboard::code::Q;
    v["R"] = (int)keyboard::code::R;
    v["S"] = (int)keyboard::code::S;
    v["T"] = (int)keyboard::code::T;
    v["U"] = (int)keyboard::code::U;
    v["V"] = (int)keyboard::code::V;
    v["W"] = (int)keyboard::code::W;
    v["X"] = (int)keyboard::code::X;
    v["Y"] = (int)keyboard::code::Y;
    v["Z"] = (int)keyboard::code::Z;
    v["0"] = (int)keyboard::code::Key0;
    v["1"] = (int)keyboard::code::Key1;
    v["2"] = (int)keyboard::code::Key2;
    v["3"] = (int)keyboard::code::Key3;
    v["4"] = (int)keyboard::code::Key4;
    v["5"] = (int)keyboard::code::Key5;
    v["6"] = (int)keyboard::code::Key6;
    v["7"] = (int)keyboard::code::Key7;
    v["8"] = (int)keyboard::code::Key8;
    v["9"] = (int)keyboard::code::Key9;
    v["F1"] = (int)keyboard::code::F1;
    v["F2"] = (int)keyboard::code::F2;
    v["F3"] = (int)keyboard::code::F3;
    v["F4"] = (int)keyboard::code::F4;
    v["F5"] = (int)keyboard::code::F5;
    v["F6"] = (int)keyboard::code::F6;
    v["F7"] = (int)keyboard::code::F7;
    v["F8"] = (int)keyboard::code::F8;
    v["F9"] = (int)keyboard::code::F9;
    v["F10"] = (int)keyboard::code::F10;
    v["F11"] = (int)keyboard::code::F11;
    v["F12"] = (int)keyboard::code::F12;
    v["UP"] = (int)keyboard::code::UP;
    v["DOWN"] = (int)keyboard::code::DOWN;
    v["LEFT"] = (int)keyboard::code::LEFT;
    v["RIGHT"] = (int)keyboard::code::RIGHT;
    v["ESCAPE"] = (int)keyboard::code::ESCAPE;
    v["SPACE"] = (int)keyboard::code::SPACE;
    v["ENTER"] = (int)keyboard::code::RETURN;
    v["BACKSPACE"] = (int)keyboard::code::BACKSPACE;
    v["TAB"] = (int)keyboard::code::TAB;
    v["LSHIFT"] = (int)keyboard::code::LSHIFT;
    v["RSHIFT"] = (int)keyboard::code::RSHIFT;
    v["LCTRL"] = (int)keyboard::code::LCTRL;
    v["RCTRL"] = (int)keyboard::code::RCTRL;
    v["ALT"] = (int)keyboard::code::ALTERASE;
    v["LCTRL"] = (int)keyboard::code::LCTRL;
    v["RCTRL"] = (int)keyboard::code::RCTRL;
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
    v["LEFT"] = (int)mouse::code::LEFT;
    v["RIGHT"] = (int)mouse::code::RIGHT;
    v["MIDDLE"] = (int)mouse::code::MIDDLE;
    v["X1"] = (int)mouse::code::X1;
    v["X2"] = (int)mouse::code::X2;
  }
  {
    auto v = lua.create_table("gamepad");
    v["is_pressed"] = &gamepad::is_pressed;
    v["is_released"] = &gamepad::is_released;
    v["is_down"] = &gamepad::is_down;
    v["get_left_stick"] = &gamepad::get_left_stick;
    v["get_right_stick"] = &gamepad::get_right_stick;
    v["is_connected"] = &gamepad::is_connected;
    v["INVALID"] = (int)gamepad::code::INVALID;
    v["A"] = (int)gamepad::code::A;
    v["B"] = (int)gamepad::code::B;
    v["X"] = (int)gamepad::code::X;
    v["Y"] = (int)gamepad::code::Y;
    v["BACK"] = (int)gamepad::code::BACK;
    v["GUIDE"] = (int)gamepad::code::GUIDE;
    v["START"] = (int)gamepad::code::START;
    v["LEFTSTICK"] = (int)gamepad::code::LEFTSTICK;
    v["RIGHTSTICK"] = (int)gamepad::code::RIGHTSTICK;
    v["LEFTSHOULDER"] = (int)gamepad::code::LEFTSHOULDER;
    v["RIGHTSHOULDER"] = (int)gamepad::code::RIGHTSHOULDER;
    v["DPAD_UP"] = (int)gamepad::code::DPAD_UP;
    v["DPAD_DOWN"] = (int)gamepad::code::DPAD_DOWN;
    v["DPAD_LEFT"] = (int)gamepad::code::DPAD_LEFT;
    v["DPAD_RIGHT"] = (int)gamepad::code::DPAD_RIGHT;
    v["MISC1"] = (int)gamepad::code::MISC1;
    v["PADDLE1"] = (int)gamepad::code::PADDLE1;
    v["PADDLE2"] = (int)gamepad::code::PADDLE2;
    v["PADDLE3"] = (int)gamepad::code::PADDLE3;
    v["PADDLE4"] = (int)gamepad::code::PADDLE4;
    v["TOUCHPAD"] = (int)gamepad::code::TOUCHPAD;
  }
}
} // namespace sinen
