#include "../audio/sound_system.hpp"
#include "../render/render_system.hpp"
#include "../texture/texture_system.hpp"
#include "math/vector3.hpp"
#include "script_system.hpp"
#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <functional>
#include <input/input.hpp>
#include <io/data_stream.hpp>
#include <main/main.hpp>
#include <math/point2.hpp>
#include <math/random.hpp>
#include <model/model.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <sol/sol.hpp>

#include "register_script.hpp"

namespace sinen {
void register_generator(sol::state &v) {

  v["require"] = [&](const std::string &str) -> sol::object {
    return v.require_script(
        str, data_stream::open_as_string(asset_type::Script, str + ".lua"));
  };
  v["texture"] = []() -> texture { return texture(); };
  v["font"] = []() -> font { return font(); };
  v["DEFAULT_FONT"] = "mplus/mplus-1p-medium.ttf";
  v["vector3"] = [](float x, float y, float z) -> vector3 {
    return vector3(x, y, z);
  };
  v["vector2"] = [](float x, float y) -> vector2 { return vector2(x, y); };
  v["point2i"] = [](int x, int y) -> point2i { return point2i(x, y); };
  v["quaternion"] = [](sol::this_state s) -> quaternion {
    return quaternion();
  };
  v["color"] = [](float r, float g, float b, float a) -> color {
    return color(r, g, b, a);
  };
  v["model"] = []() -> model { return model(); };
  v["music"] = []() -> music { return music(); };
  v["sound"] = []() -> sound { return sound(); };
  v["set_skybox_texture"] = [&](texture tex) -> void {
    render_system::set_skybox_texture(tex);
  };
  v["aabb"] = []() -> aabb { return aabb(); };
}
} // namespace sinen
