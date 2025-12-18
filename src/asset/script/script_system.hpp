#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <memory>
#include <string_view>

#include "script_backend.hpp"

namespace sinen {

enum class ScriptType { Lua };
class ScriptSystem {
public:
  static bool initialize(const ScriptType &type);
  static void shutdown();

  static void runScene(std::string_view sceneName);
  static void updateScene();
  static void drawScene();

  static ScriptType getType() { return type; }

private:
  static std::unique_ptr<IScriptBackend> script;
  static ScriptType type;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP