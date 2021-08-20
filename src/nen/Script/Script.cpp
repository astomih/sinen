#include <nen.hpp>
namespace nen
{
	void Script::Create()
	{
		instance = std::make_unique<Script>();
		lua.open_libraries(sol::lib::base, sol::lib::package);
		lua.new_usertype<Vector3>(
			// type name
			"vec3",
			// constructor
			sol::constructors<sol::types<>, sol::types<float, float, float>>(),
			// data member
			"x", &Vector3::x,
			"y", &Vector3::y,
			"z", &Vector3::z);
		lua.new_usertype<Quaternion>(
			// type name
			"quat",
			// constructor
			sol::constructors<sol::types<>, sol::types<Vector3, float>>(),
			// member function
			"concatenate", &Quaternion::Concatenate,
			// data member
			"x", &Quaternion::x,
			"y", &Quaternion::y,
			"z", &Quaternion::z);
	}

	void Script::DoScript(std::string_view fileName)
	{
		lua.script_file(fileName.data());
	}

}
