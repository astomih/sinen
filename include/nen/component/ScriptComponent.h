#pragma once
class Component;
class Script;
#include <string>
#include "../nen.hpp"
namespace nen
{
	class ScriptComponent : public Component
	{
	public:
		ScriptComponent(class Actor &owner, int updateOrder = 100);
		void DoScript(std::string_view funcName);

	private:
		const Script &script;
	};
}
