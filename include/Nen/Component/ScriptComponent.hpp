#pragma once
#include "../Script/Script.hpp"
#include "Component.hpp"
#include <string>
namespace nen {
class script_component : public base_component {
public:
  script_component(class base_actor &owner, int updateOrder = 100);
  void Update(float dt) override;
  void DoScript(std::string_view funcName);

private:
  float delta_time;
  script_system &m_script;
};
} // namespace nen