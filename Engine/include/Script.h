#pragma once
#include <memory>
#include <string>
#include <functional>
#include <sol/sol.hpp>

namespace nen
{
	class Script
	{
	public:
		Script() {}
		static void Create();
		static const Script &Get() { return *instance; }
		static sol::state &GetSolState() { return lua; }
		static void DoScript(std::string_view fileName);

	private:
		inline static std::unique_ptr<nen::Script> instance;
		inline static sol::state lua;
	};
}
