#pragma once
#include <memory>
#include <string>

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
  class scene &get_current_scene() {
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
  class camera &get_camera() {
    return *m_camera;
  }
  class random &get_random() {
    return *m_random;
  }
  void change_scene(std::string scene_number);
  std::string get_current_scene_number() { return m_scene_name; }

private:
  void loop();
  std::unique_ptr<class window> m_window;
  std::unique_ptr<class renderer> m_renderer;
  std::unique_ptr<class scene> m_current_scene;
  std::unique_ptr<class scene> m_next_scene;
  std::unique_ptr<class input_system> m_input_system;
  std::unique_ptr<class sound_system> m_sound_system;
  std::unique_ptr<class script_system> m_script_system;
  std::unique_ptr<class texture_system> m_texture_system;
  std::unique_ptr<class camera> m_camera;
  std::unique_ptr<class random> m_random;

  std::string m_scene_name = "main";
};
bool initialize();
void launch();
class window &get_window();
class renderer &get_renderer();
class input_system &get_input();
class scene &get_current_scene();
class sound_system &get_sound();
class script_system &get_script();
class texture_system &get_texture();
class camera &get_camera();
class random &get_random();
void change_scene(std::string scene_number);
std::string get_current_scene_number();
} // namespace nen