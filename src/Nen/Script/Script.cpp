#include <Nen.hpp>
#include <sol/sol.hpp>
namespace nen {
static sol::state lua = sol::state();
void script::Create() {
  instance = std::make_unique<script>();
  lua.open_libraries(sol::lib::base, sol::lib::package);
  lua.new_usertype<vector3>(
      // type name
      "vec3",
      // constructor
      sol::constructors<sol::types<>, sol::types<float, float, float>>(),
      // data member
      "x", &vector3::x, "y", &vector3::y, "z", &vector3::z);
  lua.new_usertype<quaternion>(
      // type name
      "quat",
      // constructor
      sol::constructors<sol::types<>, sol::types<vector3, float>>(),
      // member function
      "concatenate", &quaternion::Concatenate,
      // data member
      "x", &quaternion::x, "y", &quaternion::y, "z", &quaternion::z);
}

void script::DoScript(std::string_view fileName) {
  lua.script_file(fileName.data());
}

sol::state *script::GetSolState() { return &lua; }

} // namespace nen
