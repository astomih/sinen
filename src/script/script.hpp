#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <core/data/string.hpp>

namespace sinen {

class Script {
public:
  static bool initialize();
  static void shutdown();

  static void executeScene();
  static bool hasToReloadScene();

  static void callUpdate();
  static void callDraw();

  static void clearRequireCache();

  static void load(StringView filePath);
  static String getFileName();
  static String getBaseDirectory();
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP
