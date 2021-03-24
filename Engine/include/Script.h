#pragma once
#include <memory>
#include <string>
#include <functional>

namespace nen
{
	class Script {
	public:
		static bool Init();
		static void LoadModule();
		static void LoadFunc();
		template<class T, class U>
		static void CreateModule(std::function<T(U)> func, const std::string& name);
	private:
		Script() {}
		static Script* pScript;
		static bool isInited;
	};
}
