#include <Nen.hpp>
#include <sol/sol.hpp>
#include <string>
#include <string_view>
namespace nen {
script_component::script_component(base_actor &owner,
                                   std::string_view identification)
    : base_component(owner),
      m_script(owner.GetScene().get_manager().get_script_system()) {
  sol::state &lua = m_script.get_sol_state();
  lua[identification.data()] = &mOwner;
}
void script_component::DoScript(std::string_view funcName) {
  sol::state &lua = m_script.get_sol_state();

  lua["delta_time"] = delta_time;

  lua.script_file(funcName.data());
  script_name = funcName;
}
void script_component::Update(float dt) {
  delta_time = dt;
  DoScript(script_name);
}
} // namespace nen
