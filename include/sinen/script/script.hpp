#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <core/data/string.hpp>

namespace sinen {

class Script {
public:
  static bool initialize(bool isScriptDebug);
  static void shutdown();

  static void executeScene();
  static bool hasToReloadScene();

  static void callUpdate();
  static void callDraw();
  static void receiveDroppedFile(StringView path);

  static void clearRequireCache();

  static bool load_plugin(StringView pluginName);
  static String getPluginError();

  // Compatibility aliases. Native modules now use the same current-directory
  // plugin policy as load_plugin.
  static bool loadNativeModule(StringView filePath);
  static String getNativeModuleError();

  static void load(StringView filePath);
  static String getFileName();
  static String getBaseDirectory();
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP
