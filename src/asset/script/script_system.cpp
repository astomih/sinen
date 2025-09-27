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
local texture = sn.Texture()
local draw2d = sn.Draw2D(texture)
local font = sn.Font()
font:load(96)
font:render_text(texture, "NO DATA", sn.Color(1, 1, 1, 1))
draw2d.scale = texture:size()
function update()
end
function draw()
  sn.Graphics.draw2d(draw2d)
end
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