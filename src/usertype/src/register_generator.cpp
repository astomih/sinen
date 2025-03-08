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
#include <time/timer.hpp>

#include "register_script.hpp"

namespace sinen {
void register_generator(sol::state &v) {

  v["require"] = [&](const std::string &str) -> sol::object {
    return v.require_script(
        str, DataStream::open_as_string(AssetType::Script, str + ".lua"));
  };
  v["Texture"] = []() -> Texture { return Texture(); };
  v["RenderTexture"] = []() -> RenderTexture { return RenderTexture(); };
  v["Font"] = []() -> Font { return Font(); };
  v["Vector3"] = [](float x, float y, float z) -> Vector3 {
    return Vector3(x, y, z);
  };
  v["Vector2"] = [](float x, float y) -> Vector2 { return Vector2(x, y); };
  v["Point2i"] = [](int x, int y) -> Point2i { return Point2i(x, y); };
  v["Point2f"] = [](float x, float y) -> Point2f { return Point2f(x, y); };
  v["Quaternion"] = [](sol::this_state s) -> Quaternion {
    return Quaternion();
  };
  v["Color"] = [](float r, float g, float b, float a) -> Color {
    return Color(r, g, b, a);
  };
  v["Model"] = []() -> Model { return Model(); };
  v["Music"] = []() -> Music { return Music(); };
  v["Sound"] = []() -> Sound { return Sound(); };
  v["AABB"] = []() -> AABB { return AABB(); };
  v["Timer"] = []() -> Timer { return Timer(); };
  v["Shader"] = []() -> Shader { return Shader(); };
  v["UniformData"] = []() -> UniformData { return UniformData(); };
  v["RenderPipeline2D"] = []() -> RenderPipeline2D {
    return RenderPipeline2D();
  };
  v["RenderPipeline3D"] = []() -> RenderPipeline3D {
    return RenderPipeline3D();
  };
}
} // namespace sinen
