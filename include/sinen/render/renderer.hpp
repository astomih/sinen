#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../color/color.hpp"
#include "../instancing/instancing.hpp"
#include "../math/math.hpp"
#include "../math/matrix4.hpp"
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"
#include "../texture/texture_type.hpp"
#include "../vertex/vertex_array.hpp"
#include "graphics_api.hpp"

namespace sinen {

class renderer {
public:
  static graphics_api get_graphics_api();

  static void unload_data();

  static void render();

  static void draw2d(const std::shared_ptr<drawable> draw_object);
  static void draw3d(const std::shared_ptr<drawable> draw_object);

  static void add_vertex_array(const vertex_array &vArray,
                               std::string_view name);
  static void update_vertex_array(const vertex_array &vArray,
                                  std::string_view name);

  static void add_instancing(const instancing &_instancing);

  static void set_clear_color(const color &color);

  static color clear_color();

  static void set_skybox(texture _skybox_texture);

  static texture skybox();

  static void toggle_show_imgui();
  static bool is_show_imgui();

  static void load_shader(const shader &shaderinfo);
  static void unload_shader(const shader &shaderinfo);

  static std::list<std::function<void()>> &get_imgui_function();

  static void add_imgui_function(std::function<void()> function);
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
