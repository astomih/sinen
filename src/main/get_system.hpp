#pragma once
#include "../audio/sound_system.hpp"
#include "../event/event_system.hpp"
#include "../input/input_system.hpp"
#include "../math/random_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../texture/texture_system.hpp"
#include "../window/window_system.hpp"

namespace sinen {
class window_system &get_window();
class render_system &get_renderer();
class input_system &get_input();
class sound_system &get_sound();
class script_system &get_script();
class texture_system &get_texture();
class camera &get_camera();
class random_system &get_random();
class event_system &get_event();
} // namespace sinen
