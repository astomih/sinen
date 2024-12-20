#ifndef SINEN_COMPONENT_HPP
#define SINEN_COMPONENT_HPP
#include <cstdint>
#include <memory>
#include <string>
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
  component(class Actor &owner);
  /**
   * @brief Copy constructor is deleted
   *
   */
  component(const component &) = delete;
  /**
   * @brief Destroy the component object
   *
   */
  virtual ~component() = default;
  /**
   * @brief Update the component
   *
   * @param delta_time delta time
   */
  virtual void update(float delta_time) {}
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
  Actor &get_actor() { return m_owner; }

protected:
  // Owning actor
  Actor &m_owner;
};
} // namespace sinen
#endif // !SINEN_COMPONENT_HPP