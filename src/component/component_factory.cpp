// internal
#include <component/component_factory.hpp>

namespace sinen {
component_factory::component_factory() = default;
component_factory::~component_factory() = default;
std::shared_ptr<component> component_factory::create(const std::string &name,
                                                     Actor &owner) {
  if (m_component_map.contains(name)) {
    return m_component_map[name](owner);
  }
  return nullptr;
}
std::vector<std::string> component_factory::get_component_names() {
  std::vector<std::string> names;
  for (const auto &pair : m_component_map) {
    names.push_back(pair.first);
  }
  return names;
}

} // namespace sinen