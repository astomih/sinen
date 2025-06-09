#ifndef SINEN_SCENE_HPP
#define SINEN_SCENE_HPP
#include <memory>
#include <string>

#include "../camera/camera.hpp"
#include <glm/vec2.hpp>

namespace sinen {
/**
 * @brief Base of scene class
 *
 */
class Scene {
public:
  /**
   * @brief Scene implementation
   *
   */
  class implements {
  public:
    implements() = default;
    virtual ~implements() = default;
    virtual void setup() {}
    virtual void update(float delta_time) {}
    virtual void terminate() {}
  };
  /**
   * @brief Game state
   *
   */
  enum class state { running, paused, quit };
  /**
   * @brief is running scene
   *
   * @return true
   * @return false
   */
  static bool IsRunning();
  /**
   * @brief Get the State object
   *
   * @return game_state
   */
  static const state &GetState();
  /**
   * @brief Set the State object
   *
   * @param state
   */
  static void SetState(const state &state);

  /**
   * @brief Reset scene
   *
   */
  static void Reset();
  /**
   * @brief Get the current scene name
   *
   * @return std::string
   */
  static std::string GetCurrentName();
  /**
   * @brief Change scene
   *
   * @param scene_file_name scene file name in Lua
   */
  static void Change(const std::string &scene_file_name);
  /**
   * @brief Change implementation of scene
   *
   * @tparam Implements scene implementation class
   */
  template <class Implements> static void change_implements() {
    change_impl(std::make_unique<Implements>());
  }
  static void set_run_script(bool is_run);

  static Camera &GetCamera() { return m_main_camera; }

  /**
   * @brief Set the screen size
   *
   * @param size
   */
  static void Resize(const glm::vec2 &size) { m_screen_size = size; }
  /**
   * @brief Get the screen size
   *
   * @return const vector2&
   */
  static const glm::vec2 &Size() { return m_screen_size; }
  static const glm::vec2 Half() {
    glm::vec2 c = m_screen_size;
    c /= 2.f;
    return c;
  }
  static glm::vec2 Ratio();
  static glm::vec2 InvRatio();

  static float dT();

private:
  // main camera
  static Camera m_main_camera;
  static glm::vec2 m_screen_size;
  static void change_impl(std::unique_ptr<implements> impl);
};
} // namespace sinen
#endif // !SINEN_SCENE_HPP
