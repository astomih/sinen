#pragma once
#include <memory>
#include <string>
#include <functional>

namespace sol
{
	class state;
}

namespace nen
{
	class Script
	{
	public:
		Script() = default;
		static void Create();
		static class sol::state* GetSolState();
		static const Script &Get() { return *instance; }
		static void DoScript(std::string_view fileName);
		

	private:
		static inline std::unique_ptr<Script> instance;
	};
}
