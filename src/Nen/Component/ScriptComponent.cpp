#include <Nen.hpp>
#include <sol/sol.hpp>
namespace nen {
script_component::script_component(base_actor &owner, int updateOrder)
    : base_component(owner),
      m_script(owner.GetScene().get_manager().get_script_system()) {}
void script_component::DoScript(std::string_view funcName) {
  sol::state &lua = m_script.get_sol_state();

  auto &_actor = this->GetActor();
  lua["delta_time"] = delta_time;
  lua["position"] = this->GetActor().GetPosition();
  lua["rotate"] = this->GetActor().GetRotation();
  lua["set_position"] = [&](vector3 pos) { _actor.SetPosition(pos); };
  lua["set_rotation"] = [&](quaternion rot) { _actor.SetRotation(rot); };

  m_script.DoScript(funcName);
}
void script_component::Update(float dt) { delta_time = dt; }
} // namespace nen
