#include "get_system.hpp"
#include <camera/camera.hpp>
#include <input/input.hpp>
#include <logger/logger.hpp>
#include <main/main.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <script/script.hpp>
#include <utility/singleton.hpp>
#include <window/window.hpp>

#include "main_system.hpp"

namespace sinen {
window_system &get_window() { return singleton<window_system>::get(); }
render_system &get_renderer() { return singleton<render_system>::get(); }
input_system &get_input() { return singleton<input_system>::get(); }
scene &get_scene() { return singleton<main_system>::get().get_scene(); }
sound_system &get_sound() { return singleton<sound_system>::get(); }
texture_system &get_texture() { return singleton<texture_system>::get(); }
camera &get_camera() { return singleton<camera>::get(); }
random_system &get_random() { return singleton<random_system>::get(); }
script_system &get_script() { return singleton<script_system>::get(); }
event_system &get_event() { return singleton<event_system>::get(); }
} // namespace sinen