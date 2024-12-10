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
   * @return std::shared_ptr<component> Component
   */
  std::shared_ptr<component> create(const std::string &name, class Actor &owner);
  /**
   * @brief Register a component
   *
   * @tparam T Component type
   * @param name Component name
   */
  template <typename T> void register_component(const std::string &name) {
    m_component_map[name] = [](Actor &owner) {
      return std::make_shared<T>(owner);
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
  std::unordered_map<std::string,
                     std::function<std::shared_ptr<component>(class Actor &)>>
      m_component_map;
};
} // namespace sinen
#endif // !SINEN_COMPONENT_FACTORY