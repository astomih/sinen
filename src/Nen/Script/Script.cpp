#include <Nen.hpp>
#include <sol/sol.hpp>
namespace nen {
script_system::script_system() = default;
script_system::~script_system() = default;
class script_system::implement {
public:
  implement() { lua = sol::state(); }
  sol::state &get_state() { return lua; }

private:
  sol::state lua;
};
bool script_system::initialize() {
  m_impl = std::make_unique<implement>();
  auto &lua = m_impl->get_state();

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
  return true;
}

void script_system::DoScript(std::string_view fileName) {
  m_impl->get_state().script_file(fileName.data());
}

sol::state &script_system::get_sol_state() { return m_impl->get_state(); }

} // namespace nen
