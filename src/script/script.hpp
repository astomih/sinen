#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <core/data/string.hpp>

namespace sinen {

class Script {
public:
  static bool initialize();
  static void shutdown();

  static void runScene();
  static void updateScene();
  static void drawScene();

  static bool hasToReload();
  static void doneReload();

  static void setSceneName(StringView name);
  static String getSceneName();
  static void load(StringView filePath, StringView baseDirPath = "");
  static void setBasePath(StringView path);
  static String getBasePath();

private:
  inline static String sceneName = "main";
  inline static String basePath = ".";
  inline static bool reload = true;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP