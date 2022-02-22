#pragma once

#include "../Render/Renderer.hpp"
#include "../Window/Window.hpp"

#include "../Input/InputSystem.hpp"

#include "../Audio/SoundSystem.hpp"
#include "../Render/graphics_api.hpp"
#include "../Scene/Scene.hpp"
#include "../Script/Script.hpp"
#include "../Utility/dynamic_handler.hpp"
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
  void initialize();

  void change_scene(std::unique_ptr<base_scene> next_scene) {
    m_current_scene->Quit();
    m_next_scene = std::move(next_scene);
  }

  /**
   * @brief launch engine
   *
   */
  void launch(std::unique_ptr<class base_scene>);

  class window &get_window() {
    return *m_window;
  }
  class renderer &get_renderer() {
    return *m_renderer;
  }
  class input_system &get_input_system() {
    return *m_input_system;
  }
  class base_scene &get_current_scene() {
    return *m_current_scene;
  }
  class sound_system &get_sound_system() {
    return *m_sound_system;
  }
  class script_system &get_script_system() {
    return *m_script_system;
  }

private:
  void loop();
  std::unique_ptr<class window> m_window;
  std::unique_ptr<class renderer> m_renderer;
  std::unique_ptr<class base_scene> m_current_scene;
  std::unique_ptr<class base_scene> m_next_scene;
  std::unique_ptr<class input_system> m_input_system;
  std::unique_ptr<class sound_system> m_sound_system;
  std::unique_ptr<class script_system> m_script_system;
};
} // namespace nen