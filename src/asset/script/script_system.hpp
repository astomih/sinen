#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include "script_backend.hpp"
#include <core/data/string.hpp>

namespace sinen {

enum class ScriptType { Lua };
class ScriptSystem {
public:
  static bool initialize(const ScriptType &type);
  static void shutdown();

  static void runScene(StringView sceneName);
  static void updateScene();
  static void drawScene();

  static ScriptType getType() { return type; }

private:
  static UniquePtr<IScriptBackend> script;
  static ScriptType type;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP