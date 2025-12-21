#include "../../main_system.hpp"
#include <asset/script/script.hpp>

#include <string>

namespace sinen {
void Script::load(StringView filePath, StringView baseDirPath) {
  MainSystem::change(filePath, baseDirPath);
}

} // namespace sinen