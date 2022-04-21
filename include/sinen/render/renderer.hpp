#pragma once
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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nen {

class renderer {
public:
  class Interface {
  public:
    Interface() = default;
    virtual ~Interface() {}

    virtual void Initialize() {}
    virtual void Shutdown() {}
    virtual void Render() {}
    virtual void draw2d(std::shared_ptr<class draw_object> sprite) {}
    virtual void draw3d(std::shared_ptr<class draw_object> sprite) {}
    virtual void AddVertexArray(const vertex_array &vArray,
                                std::string_view name) {}
    virtual void UpdateVertexArray(const vertex_array &vArray,
                                   std::string_view name) {}

    virtual void add_instancing(const instancing &_instancing) {}

    virtual void LoadShader(const shader &shaderInfo) {}
    virtual void UnloadShader(const shader &shaderInfo) {}
  };
  renderer(class manager &_manager);
  void initialize(graphics_api api);
  ~renderer() = default;

  graphics_api GetGraphicsAPI() { return RendererAPI; }

  void Shutdown();
  void UnloadData();

  void render();

  void draw2d(const std::shared_ptr<draw_object> draw_object);
  void draw3d(const std::shared_ptr<draw_object> draw_object);

  void AddVertexArray(const vertex_array &vArray, std::string_view name);
  void UpdateVertexArray(const vertex_array &vArray, std::string_view name);

  void add_instancing(const instancing &_instancing);

  void SetClearColor(const color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }

  color GetClearColor() { return this->clearColor; }

  class window &GetWindow();

  void toggleShowImGui() { showImGui = !showImGui; }
  bool isShowImGui() { return showImGui; }

  void LoadShader(const shader &shaderinfo);
  void UnloadShader(const shader &shaderinfo);

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
  std::unique_ptr<Interface> m_renderer;
  graphics_api RendererAPI;
  bool showImGui;
  std::vector<std::function<void()>> m_imgui_function;
};

} // namespace nen