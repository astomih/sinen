#ifndef SINEN_ACTOR_HPP
#define SINEN_ACTOR_HPP
#include "../component/component.hpp"
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"
#include <cstdint>
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
   */
  actor();

  /**
   * @brief Destroy the base actor object
   *
   */
  virtual ~actor();
  /**
   * @brief Update Actor
   * @details This function is called every frame by the scene.
   * Components are updated in the order they were added.
   * @param deltaTime Delta time of the frame
   */
  virtual void update(float delta_time);
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
  const vector3 &get_rotation() const { return m_rotation; }
  /**
   * @brief Set the rotation object
   *
   * @param rotation  Quaternion
   */
  void set_rotation(const vector3 &rotation) { m_rotation = rotation; }
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
    return vector3();
    // return vector3::transform(up, m_rotation);
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
  // Get state
  bool is_active() const { return m_state == state::active; }
  bool is_paused() const { return m_state == state::paused; }
  bool is_dead() const { return m_state == state::dead; }
  template <class Component, class... Args>
  Component &create_component(Args &&...args) {
    auto ptr = std::make_shared<Component>(*this, std::forward<Args>(args)...);
    Component &ref = *reinterpret_cast<Component *>(ptr.get());
    add_component(ptr);
    return ref;
  }
  using component_ptr = std::shared_ptr<component>;
  void add_component(component_ptr comp);
  void remove_component(component_ptr comp);
  std::vector<component_ptr> &get_components() { return m_components; }

  /**
   * @brief Get the name object
   *
   * @return const std::string& Name of the actor
   */
  const std::string &get_name() const { return m_name; }
  /**
   * @brief Set the name object
   *
   * @param name Name of the actor
   */
  void set_name(const std::string &name) { m_name = name; }
  /**
   * @brief Get the script name object
   *
   * @return const std::string& Script name of the actor
   */
  const std::string &get_script_name() const { return m_script_name; }
  /**
   * @brief Set the script name object
   *
   * @param script_name Script name of the actor
   */
  void set_script_name(const std::string &script_name) {
    m_script_name = script_name;
  }

private:
  std::vector<component_ptr> m_components;
  state m_state;
  vector3 m_position;
  vector3 m_rotation;
  vector3 m_scale;
  std::string m_name;
  std::string m_script_name;
};
} // namespace sinen
#endif // !SINEN_ACTOR_HPP
