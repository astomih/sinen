#pragma once
#include "../Script/Script.hpp"
#include <string>
namespace nen {
class script_component : public base_component {
public:
  script_component(class base_actor &owner, int updateOrder = 100);
  void DoScript(std::string_view funcName);

private:
  const script &script;
};
} // namespace nen