#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <core/data/string.hpp>

namespace sinen {

class ScriptSystem {
public:
  static bool initialize();
  static void shutdown();

  static void runScene();
  static void updateScene();
  static void drawScene();

  static bool hasToReload() { return reload; }
  static void doneReload() { reload = false; }
  static bool hasToQuit() { return sceneName.empty(); }

  static void setSceneName(StringView name) {
    sceneName = name;
    reload = !sceneName.empty();
  }
  static String getSceneName() { return sceneName; }
  static void setBasePath(StringView path) {
    basePath = path;
    reload = true;
  }
  static String getBasePath() { return basePath; }

private:
  inline static String sceneName = "main";
  inline static String basePath = ".";
  inline static bool reload = true;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP