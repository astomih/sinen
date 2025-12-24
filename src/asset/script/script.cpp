#include "script_system.hpp"
#include <asset/script/script.hpp>

#include <string>

namespace sinen {
void Script::load(StringView filePath, StringView baseDirPath) {
  ScriptSystem::setSceneName(filePath);
  ScriptSystem::setBasePath(baseDirPath);
}

} // namespace sinen