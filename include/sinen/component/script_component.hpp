#ifndef SCRIPT_COMPONENT_HPP
#define SCRIPT_COMPONENT_HPP
#include "component.hpp"
namespace sinen {
class script_component : public component {
public:
  script_component(class actor& owner);
  ~script_component();
  /**
   * @brief update Update component
   * @param delta_time Scene delta time
   */
  void update(float delta_time)override;

  void load_script(std::string_view path);
private:
  std::string script_raw_str;

};
} // namespace sinen

#endif // SCRIPT_COMPONENT_HPP
