#ifndef SINEN_COMPONENT_HPP
#define SINEN_COMPONENT_HPP
#include <cstdint>
#include <memory>
namespace sinen {
/**
 * @brief Component
 *
 */
class component {
public:
  /**
   * @brief Construct a new component object
   *
   * @param owner The actor that owns this component
   */
  component(class actor &owner);
  /**
   * @brief Construct a new component object
   *
   */
  component(const component &) = delete;
  /**
   * @brief Destroy the component object
   *
   */
  virtual ~component();
  /**
   * @brief Update the component
   *
   * @param delta_time delta time
   */
  virtual void update(float delta_time);
  /**
   * @brief Get the name object
   *
   * @return std::string
   */
  virtual std::string get_name() const { return "unknown"; }
  /**
   * @brief Get the actor object
   *
   * @return actor& actor
   */
  actor &get_actor() { return m_owner; }

protected:
  // Owning actor
  actor &m_owner;
};
} // namespace sinen
#endif // !SINEN_COMPONENT_HPP