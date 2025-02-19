#include <audio/music.hpp>
#include <audio/sound.hpp>
#include <camera/camera.hpp>
#include <font/font.hpp>
#include <io/data_stream.hpp>
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
        str, DataStream::open_as_string(AssetType::Script, str + ".lua"));
  };
  v["texture"] = []() -> Texture { return Texture(); };
  v["font"] = []() -> Font { return Font(); };
  v["vector3"] = [](float x, float y, float z) -> Vector3 {
    return Vector3(x, y, z);
  };
  v["vector2"] = [](float x, float y) -> Vector2 { return Vector2(x, y); };
  v["point2i"] = [](int x, int y) -> Point2i { return Point2i(x, y); };
  v["point2f"] = [](float x, float y) -> Point2f { return Point2f(x, y); };
  v["quaternion"] = [](sol::this_state s) -> Quaternion {
    return Quaternion();
  };
  v["color"] = [](float r, float g, float b, float a) -> Color {
    return Color(r, g, b, a);
  };
  v["model"] = []() -> Model { return Model(); };
  v["music"] = []() -> Music { return Music(); };
  v["sound"] = []() -> Sound { return Sound(); };
  v["set_skybox_texture"] = [&](Texture tex) -> void {
    Renderer::set_skybox(tex);
  };
  v["aabb"] = []() -> AABB { return AABB(); };
}
} // namespace sinen
