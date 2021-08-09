#include <nen.hpp>
namespace nen
{
	ScriptComponent::ScriptComponent(Actor &owner, int updateOrder)
		: Component(owner), script(Script::Get())
	{
	}
	void ScriptComponent::DoScript(std::string_view funcName)
	{
		auto &lua = script.GetSolState();
		auto &actor = this->GetActor();
		lua["pos"] = this->GetActor().GetPosition();
		lua["rotate"] = this->GetActor().GetRotation();
		lua["setPosition"] = [&](Vector3f pos) { actor.SetPosition(pos); };
		lua["setRotation"] = [&](Quaternion rot) { actor.SetRotation(rot); };

		Script::DoScript(funcName);
	}
}
