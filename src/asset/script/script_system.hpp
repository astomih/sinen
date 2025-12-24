#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <core/data/string.hpp>

namespace sinen {

class ScriptSystem {
public:
  static bool initialize();
  static void shutdown();

  static void runScene(StringView sceneName);
  static void updateScene();
  static void drawScene();
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP