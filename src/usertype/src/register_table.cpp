#include "register_script.hpp"

#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

#include <input/gamepad.hpp>
#include <input/keyboard.hpp>
#include <input/mouse.hpp>
#include <math/periodic.hpp>
#include <time/time.hpp>
#include <time/timer.hpp>

namespace sinen {
void register_table(sol::state &lua) {
  {
    auto v = lua.create_table("window");
    v["name"] = &Window::name;
    v["size"] = &Window::size;
    v["center"] = Window::center;
    v["resize"] = &Window::resize;
    v["set_fullscreen"] = &Window::set_fullscreen;
    v["rename"] = &Window::rename;
    v["resized"] = &Window::resized;
  }
  {
    auto v = lua.create_table("renderer");
    v["set_skybox"] = &Renderer::set_skybox;
    v["clear_color"] = &Renderer::clear_color;
    v["set_clear_color"] = &Renderer::set_clear_color;
    v["at_render_texture_user_data"] = &Renderer::at_render_texture_user_data;
    v["set_light_lookat"] = &Renderer::set_light_look_at;
    v["set_light_ortho"] = &Renderer::set_light_ortho;
  }
  {
    auto v = lua.create_table("scene");
    v["main_camera"] = &Scene::main_camera;
    v["size"] = &Scene::size;
    v["resize"] = &Scene::resize;
    v["center"] = &Scene::center;
  }
  {
    auto v = lua.create_table("collision");
    v["aabb_aabb"] = &Collision::aabb_aabb;
  }
  {
    auto v = lua.create_table("keyboard");
    v["is_pressed"] = &Keyboard::is_pressed;
    v["is_released"] = &Keyboard::is_released;
    v["is_down"] = &Keyboard::is_down;
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
    auto v = lua.create_table("mouse");
    v["is_pressed"] = &Mouse::is_pressed;
    v["is_released"] = &Mouse::is_released;
    v["is_down"] = &Mouse::is_down;
    v["position"] = &Mouse::get_position;
    v["position_on_scene"] = &Mouse::get_position_on_scene;
    v["set_position"] = &Mouse::set_position;
    v["set_position_on_scene"] = &Mouse::set_position_on_scene;
    v["scroll_wheel"] = &Mouse::get_scroll_wheel;
    v["hide_cursor"] = &Mouse::hide_cursor;
    v["LEFT"] = (int)Mouse::LEFT;
    v["RIGHT"] = (int)Mouse::RIGHT;
    v["MIDDLE"] = (int)Mouse::MIDDLE;
    v["X1"] = (int)Mouse::X1;
    v["X2"] = (int)Mouse::X2;
  }
  {
    auto v = lua.create_table("gamepad");
    v["is_pressed"] = &GamePad::is_pressed;
    v["is_released"] = &GamePad::is_released;
    v["is_down"] = &GamePad::is_down;
    v["left_stick"] = &GamePad::get_left_stick;
    v["right_stick"] = &GamePad::get_right_stick;
    v["is_connected"] = &GamePad::is_connected;
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
    auto v = lua.create_table("periodic");
    v["sin0_1"] = [](float period, float t) {
      return Periodic::sin0_1(period, t);
    };
    v["cos0_1"] = [](float period, float t) {
      return Periodic::cos0_1(period, t);
    };
  }
  {
    auto v = lua.create_table("time");
    v["seconds"] = &Time::seconds;
    v["milli"] = &Time::milli;
  }
  {
    auto v = lua.create_table("timer");
    v["start"] = &Timer::start;
    v["stop"] = &Timer::stop;
    v["is_started"] = &Timer::is_started;
    v["set_time"] = &Timer::set_time;
    v["check"] = &Timer::check;
  }
}
} // namespace sinen
