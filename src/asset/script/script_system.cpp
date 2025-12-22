// internal
#include "script_system.hpp"
#include "script_backend.hpp"
#include <core/io/asset_io.hpp>

namespace sinen {
UniquePtr<IScriptBackend> ScriptSystem::script = nullptr;
ScriptType ScriptSystem::type = ScriptType::Lua;

bool ScriptSystem::initialize(const ScriptType &type) {
  switch (type) {
  case ScriptType::Lua:
    script = std::move(ScriptBackend::createLua());
    ScriptSystem::type = ScriptType::Lua;
    break;
  default:
    return false;
  }
  return script->initialize();
}
void ScriptSystem::shutdown() {
  if (script) {
    script->finalize();
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

void ScriptSystem::runScene(StringView sceneName) {
  if (script) {
    String source;
    switch (ScriptSystem::type) {
    case ScriptType::Lua: {
      source = AssetIO::openAsString(String(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
      }
    } break;
    default:
      break;
    }
    script->runScene(AssetIO::openAsString(String(sceneName) + ".lua"),
                     "@" + AssetIO::getFilePath(sceneName) + ".lua");
  }
}

void ScriptSystem::updateScene() {
  if (script) {
    script->update();
  }
}

void ScriptSystem::drawScene() {
  if (script) {
    script->draw();
  }
}

} // namespace sinen