#pragma once
namespace sinen {
class window_system &get_window();
class render_system &get_renderer();
class input_system &get_input();
class scene &get_current_scene();
class sound_system &get_sound();
class script_system &get_script();
class texture_system &get_texture();
class camera &get_camera();
class random_system &get_random();
} // namespace sinen
