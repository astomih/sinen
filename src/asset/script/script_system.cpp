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
  case ScriptType::Python:
    script = ScriptBackend::CreatePython();
    ScriptSystem::type = ScriptType::Python;
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
function update()
end
function draw()
    sn.Graphics.draw_text("NO DATA", sn.Vec2(0, 0), sn.Color(1, 1, 1, 1), 96, 0.0)
end
)";

static const char *nothingScenePython = R"(
import sinen as sn

def update():
    pass

def draw():
    sn.Graphics.draw_text("NO DATA", sn.Vec2(0, 0), sn.Color(1, 1, 1, 1), 96, 0.0)
)";

void ScriptSystem::RunScene(std::string_view sceneName) {
  if (script) {
    std::string source;
    switch (ScriptSystem::type) {
    case ScriptType::Lua: {
      source = AssetIO::openAsString(AssetType::Script,
                                     std::string(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
      }
    } break;
    case ScriptType::Python: {
      source = AssetIO::openAsString(AssetType::Script,
                                     std::string(sceneName) + ".py");
      if (source.empty()) {
        source = nothingScenePython;
      }
    } break;
    default:
      break;
    }
    script->RunScene(source);
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