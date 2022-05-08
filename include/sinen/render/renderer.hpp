#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
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

namespace nen {

class renderer {
public:
  renderer(class manager &_manager);
  ~renderer();
  void initialize(graphics_api api);

  graphics_api GetGraphicsAPI() { return RendererAPI; }

  void shutdown();
  void unload_data();

  void render();

  void draw2d(const std::shared_ptr<draw_object> draw_object);
  void draw3d(const std::shared_ptr<draw_object> draw_object);

  void add_vertex_array(const vertex_array &vArray, std::string_view name);
  void update_vertex_array(const vertex_array &vArray, std::string_view name);

  void add_instancing(const instancing &_instancing);

  void SetClearColor(const color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }

  color GetClearColor() { return this->clearColor; }

  class window &GetWindow();

  void toggleShowImGui() { showImGui = !showImGui; }
  bool isShowImGui() { return showImGui; }

  void load_shader(const shader &shaderinfo);
  void unload_shader(const shader &shaderinfo);

  std::vector<std::function<void()>> &get_imgui_function() {
    return m_imgui_function;
  }

  void add_imgui_function(std::function<void()> function) {
    m_imgui_function.push_back(function);
  }

  std::unique_ptr<texture> skybox_texture;

private:
  class manager &m_manager;
  void setup_shapes();
  color clearColor = palette::Black;

  // Window
  std::shared_ptr<class window> mWindow;
  // Renderer
  std::unique_ptr<class gl_renderer> m_gl_renderer;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::unique_ptr<class vk_renderer> m_vk_renderer;
#endif
  graphics_api RendererAPI;
  bool showImGui;
  std::vector<std::function<void()>> m_imgui_function;
};

} // namespace nen
#endif