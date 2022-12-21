#ifndef SINEN_COMPONENT_FACTORY
#define SINEN_COMPONENT_FACTORY
#include "component.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace sinen {
/**
 * @brief Component factory
 *
 */
class component_factory {
public:
  /**
   * @brief Construct a new component factory object
   *
   */
  component_factory();
  /**
   * @brief Destroy the component factory object
   *
   */
  ~component_factory();
  /**
   * @brief Create a component object
   *
   * @param name Component name
   * @param owner Owning actor
   * @return component* Component
   */
  component *create(const std::string &name, actor &owner);
  /**
   * @brief Register a component
   *
   * @tparam T Component type
   * @param name Component name
   */
  template <typename T> void register_component(const std::string &name) {
    m_component_map[name] = [](actor &owner) -> component * {
      return new T(owner);
    };
  }
  /**
   * @brief Unregister a component
   *
   * @param name Component name
   */
  void unregister_component(const std::string &name) {
    m_component_map.erase(name);
  }
  /**
   * @brief Get the component names object
   *
   * @return std::vector<std::string> Component names
   */
  std::vector<std::string> get_component_names();

private:
  // Component map
  std::unordered_map<std::string, std::function<component *(actor &)>>
      m_component_map;
};
} // namespace sinen
#endif // !SINEN_COMPONENT_FACTORY