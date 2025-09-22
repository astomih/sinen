#include "../../core/scene/scene_system.hpp"
#include <asset/script/script.hpp>

#include <string>

namespace sinen {
void Script::Load(const std::string &filePath, const std::string &baseDirPath) {
  SceneSystem::Change(std::string(filePath.data()),
                      std::string(baseDirPath.data()));
}

} // namespace sinen