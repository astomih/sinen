#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include <memory>
#include <string_view>

#include "script.hpp"

namespace sinen {

enum class ScriptType { Lua };
class ScriptSystem {
public:
  static bool Initialize(const ScriptType &type);
  static void Shutdown();

  static void RunScene(std::string_view sceneName);
  static void UpdateScene();
  static void DrawScene();

  static ScriptType GetType() { return type; }

private:
  static std::unique_ptr<IScript> script;
  static ScriptType type;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP