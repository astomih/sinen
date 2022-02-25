#pragma once

#include "../Render/Renderer.hpp"
#include "../Window/Window.hpp"

#include "../Input/InputSystem.hpp"

#include "../Audio/SoundSystem.hpp"
#include "../Render/graphics_api.hpp"
#include "../Scene/Scene.hpp"
#include "../Script/Script.hpp"
#include <memory>

namespace nen {
/**
 * @brief sinen manager
 *
 */
class manager {
public:
  /**
   * @brief Construct a new manager object
   *
   */
  manager() = default;
  ~manager() = default;
  manager(const manager &) = delete;
  /**
   * @brief initialize manager
   *
   */
  bool initialize();

  void change_scene(std::uint32_t scene_number);

  /**
   * @brief launch engine
   *
   */
  void launch();

  class window &get_window() {
    return *m_window;
  }
  class renderer &get_renderer() {
    return *m_renderer;
  }
  class base_scene &get_current_scene() {
    return *m_current_scene;
  }
  class input_system &get_input_system() {
    return *m_input_system;
  }
  class sound_system &get_sound_system() {
    return *m_sound_system;
  }
  class script_system &get_script_system() {
    return *m_script_system;
  }
  class texture_system &get_texture_system() {
    return *m_texture_system;
  }
  class font_system &get_font_system() {
    return *m_font_system;
  }

  std::uint32_t get_current_scene_number() { return m_scene_number; }

private:
  void loop();
  std::unique_ptr<class window> m_window;
  std::unique_ptr<class renderer> m_renderer;
  std::unique_ptr<class base_scene> m_current_scene;
  std::unique_ptr<class base_scene> m_next_scene;
  std::unique_ptr<class input_system> m_input_system;
  std::unique_ptr<class sound_system> m_sound_system;
  std::unique_ptr<class script_system> m_script_system;
  std::unique_ptr<class texture_system> m_texture_system;
  std::unique_ptr<class font_system> m_font_system;
  std::uint32_t m_scene_number = 1;
};
manager &get_manager();
class window &get_window();
class renderer &get_renderer();
class input_system &get_input_system();
class base_scene &get_current_scene();
class sound_system &get_sound_system();
class script_system &get_script_system();
class texture_system &get_texture_system();
class font_system &get_font_system();
} // namespace nen