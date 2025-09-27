#include "../../main_system.hpp"
#include <asset/script/script.hpp>

#include <string>

namespace sinen {
void Script::load(const std::string &filePath, const std::string &baseDirPath) {
  MainSystem::Change(std::string(filePath.data()),
                     std::string(baseDirPath.data()));
}

} // namespace sinen