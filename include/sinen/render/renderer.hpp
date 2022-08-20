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
  renderer();
  ~renderer();

  graphics_api get_graphics_api();

  void unload_data();

  void render();

  void draw2d(const std::shared_ptr<drawable> draw_object);
  void draw3d(const std::shared_ptr<drawable> draw_object);

  void add_vertex_array(const vertex_array &vArray, std::string_view name);
  void update_vertex_array(const vertex_array &vArray, std::string_view name);

  void add_instancing(const instancing &_instancing);

  void set_clear_color(const color &color);

  color get_clear_color();

  void set_skybox_texture(texture _skybox_texture);

  texture get_skybox_texture();

  void toggle_show_imgui();
  bool is_show_imgui();

  void load_shader(const shader &shaderinfo);
  void unload_shader(const shader &shaderinfo);

  std::list<std::function<void()>> &get_imgui_function();

  void add_imgui_function(std::function<void()> function);

private:
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
