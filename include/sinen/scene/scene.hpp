#ifndef SINEN_SCENE_HPP
#define SINEN_SCENE_HPP
#include "../actor/actor.hpp"
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../camera/camera.hpp"

namespace sinen {
/**
 * @brief Base of scene class
 *
 */
class scene {
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
  static bool is_running();
  /**
   * @brief Get the State object
   *
   * @return game_state
   */
  static const state &get_state();
  /**
   * @brief Set the State object
   *
   * @param state
   */
  static void set_state(const state &state);

  /**
   * @brief Reset scene
   *
   */
  static void reset();
  /**
   * @brief Change implementation of scene
   *
   * @tparam Implements scene implementation class
   */
  template <class Implements> static void change_implements() {
    change_impl(std::make_unique<Implements>());
  }
  static void set_run_script(bool is_run);

  static void load(std::string_view data_file_name);

  template <class Actor = actor, class... Args>
  static Actor &create_actor(Args &&...args) {
    actor *ptr = new Actor(std::forward<Args>(args)...);
    Actor &ref = *reinterpret_cast<Actor *>(ptr);
    add_actor(ptr);
    return ref;
  }
  static camera &main_camera() { return m_main_camera; }

private:
  // main camera
  static camera m_main_camera;
  static void add_actor(actor *_actor);
  static void change_impl(std::unique_ptr<implements> impl);
};
} // namespace sinen
#endif // !SINEN_SCENE_HPP
