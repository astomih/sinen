#include <Engine.hpp>
namespace nen
{
	bool Script::Init()
	{
		return true;
	}

	void Script::LoadModule()
	{
	}

	void Script::LoadFunc()
	{
	}

	template<class T, class U>
	void Script::CreateModule(std::function<T(U)> function, const std::string& name)
	{
	}


	Script* Script::pScript = nullptr;
	bool Script::isInited = false;
}
