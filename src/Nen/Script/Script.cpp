#include <Nen.hpp>
#include <functional>
#include <sol/sol.hpp>

namespace nen {
bool script_system::initialize() {
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);

  return true;
}

void script_system::DoScript(std::string_view fileName) {
  lua.script_file(asset_reader::ConvertFilePath(fileName, asset_type::Script));
}

} // namespace nen
