#ifndef SINEN_ACTOR_HPP
#define SINEN_ACTOR_HPP
#include "../component/component.hpp"
#include "../input/input.hpp"
#include "../math/matrix4.hpp"
#include "../math/quaternion.hpp"
#include "../math/vector3.hpp"
#include "utility/handler.hpp"
#include <cstdint>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>
namespace sinen {
class scene;
/**
 * @brief Base of all actors.
 *
 */
class actor {
public:
  /**
   * @brief State of the actor
   *
   */
  enum class state { active, paused, dead };

  /**
   * @brief Construct a new base actor object
   *
   * @param scene Parent scene
   */
  actor(scene &scene);

  /**
   * @brief Destroy the base actor object
   *
   */
  virtual ~actor();
  /**
   * @brief Update Actor
   *
   * @param deltaTime Delta time of the frame
   */
  virtual void update(float delta_time);
  /**
   * @brief Update component
   *
   * @param deltaTime Delta time of the frame
   */
  virtual void update_components(float delta_time);
  /**
   * @brief Get the position vector
   *
   * @return const Vector3& Position vector
   */
  const vector3 &get_position() const noexcept { return m_position; }
  /**
   * @brief Set the position vector
   *
   * @param pos Vector3
   */
  void set_position(const vector3 &pos) noexcept { m_position = pos; }
  /**
   * @brief Add to the position vector by float
   *
   * @param x X
   * @param y Y
   * @param z Z
   */
  void move(const float x, const float y, const float z) noexcept {
    m_position.x += x;
    m_position.y += y;
    m_position.z += z;
  }
  /**
   * @brief Add to the position vector by Vector3
   *
   * @param vec
   */
  void move(const vector3 &vec) noexcept { m_position += vec; }

  /**
   * @brief Get the scale vector
   *
   * @return const Vector3& Scale vector
   */
  const vector3 &get_scale() const { return m_scale; }

  /**
   * @brief Set the scale vector
   *
   * @param scale Vector3
   */
  void set_scale(const vector3 &scale) { m_scale = scale; }
  /**
   * @brief Get the rotation quaternion
   *
   * @return const Quaternion&
   */
  const quaternion &get_rotation() const { return m_rotation; }
  /**
   * @brief Set the rotation object
   *
   * @param rotation  Quaternion
   */
  void set_rotation(const quaternion &rotation) { m_rotation = rotation; }
  /**
   * @brief Get the world transform matrix
   *
   * @return const matrix4&
   */
  matrix4 get_world_matrix() const;
  /**
   * @brief Get the forward object
   *
   * @param up Up vector
   * @return vector3 Forward vector
   */
  vector3 get_forward(const vector3 &up) const {
    return vector3::transform(up, m_rotation);
  }
  /**
   * @brief Get the State object
   *
   * @return state Actor state object
   */
  state get_state() const { return m_state; }
  /**
   * @brief Set the actor state object
   *
   * @param stateb Actor state object
   */
  void set_state(state state) { m_state = state; }
  /**
   * @brief Get the parent scene object
   *
   * @return scene& parent scene
   */
  scene &get_scene() { return m_scene; }

  template <class T, typename... Args> handle_t add_component(Args &&...args) {
    return m_components.create(*this, std::forward<Args>(args)...);
  }
  template <class T> T &get_component(const handle_t &handle) {
    return m_components.get<T>(handle);
  }

  void remove_component(const handle_t &stored_value) {
    m_components.remove(stored_value);
  }

private:
  scene &m_scene;
  state m_state;
  vector3 m_position;
  quaternion m_rotation;
  vector3 m_scale;
  handler<component> m_components;
};
} // namespace sinen
#endif // !SINEN_ACTOR_HPP