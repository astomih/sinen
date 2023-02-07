#ifndef SINEN_SCENE_HPP
#define SINEN_SCENE_HPP
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../actor/actor.hpp"
#include "../camera/camera.hpp"
#include "../component/component_factory.hpp"
#include "../math/vector2.hpp"

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
  static actor &get_actor(const std::string &name);
  static camera &main_camera() { return m_main_camera; }

  static component_factory &get_component_factory() {
    return m_component_factory;
  }
  static void set_component_factory(const component_factory &factory) {
    m_component_factory = factory;
  }
  /**
   * @brief Set the screen size
   *
   * @param size
   */
  static void resize(const vector2 &size) { m_screen_size = size; }
  /**
   * @brief Get the screen size
   *
   * @return const vector2&
   */
  static const vector2 &size() { return m_screen_size; }
  static const vector2 center() {
    vector2 c = m_screen_size;
    c /= 2.f;
    return c;
  }

private:
  static component_factory m_component_factory;
  // main camera
  static camera m_main_camera;
  static vector2 m_screen_size;
  static void add_actor(actor *_actor);
  static void change_impl(std::unique_ptr<implements> impl);
};
} // namespace sinen
#endif // !SINEN_SCENE_HPP
