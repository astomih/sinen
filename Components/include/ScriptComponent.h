#pragma once
class Component;
class Script;
#include <string>
class ScriptComponent : public Component
{
public:
	ScriptComponent(class Actor& owner, int updateOrder = 100) 
		:Component(owner)
	{}
	void LoadModule(std::string_view moduleName);
	void SetArgs();
	void DoFunction(std::string_view funcName);
private:
};
