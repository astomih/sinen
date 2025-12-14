// internal
#include "script_system.hpp"
#include "script_backend.hpp"
#include <core/io/asset_io.hpp>

namespace sinen {
std::unique_ptr<IScriptBackend> ScriptSystem::script = nullptr;
ScriptType ScriptSystem::type = ScriptType::Lua;

bool ScriptSystem::Initialize(const ScriptType &type) {
  switch (type) {
  case ScriptType::Lua:
    script = ScriptBackend::CreateLua();
    ScriptSystem::type = ScriptType::Lua;
    break;
  default:
    return false;
  }
  return script->Initialize();
}
void ScriptSystem::Shutdown() {
  if (script) {
    script->Finalize();
    script.reset();
  }
}

static const char *nothingSceneLua = R"(
local font = sn.Font.new()
font:load(32)
function Update()
end

function Draw()
    sn.Graphics.drawText("NO DATA", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32, 0.0)
end
)";

void ScriptSystem::RunScene(std::string_view sceneName) {
  if (script) {
    std::string source;
    switch (ScriptSystem::type) {
    case ScriptType::Lua: {
      source = AssetIO::openAsString(std::string(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
      }
    } break;
    default:
      break;
    }
    script->RunScene(AssetIO::openAsString(std::string(sceneName) + ".lua"),
                     "@" + AssetIO::getFilePath(sceneName) + ".lua");
  }
}

void ScriptSystem::UpdateScene() {
  if (script) {
    script->Update();
  }
}

void ScriptSystem::DrawScene() {
  if (script) {
    script->Draw();
  }
}

} // namespace sinen