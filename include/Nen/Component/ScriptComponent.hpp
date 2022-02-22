#pragma once
#include "../Script/Script.hpp"
#include "Component.hpp"
#include <string>
#include <string_view>
namespace nen {
class script_component : public base_component {
public:
  script_component(class base_actor &owner, std::string_view identification);
  void Update(float dt) override;
  void DoScript(std::string_view funcName);

private:
  float delta_time;
  std::string script_name;
  script_system &m_script;
};
} // namespace nen