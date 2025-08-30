// internal
#include "script_system.hpp"
#include "script.hpp"
#include <core/io/asset_io.hpp>

namespace sinen {
std::unique_ptr<IScript> ScriptSystem::script = nullptr;
ScriptType ScriptSystem::type = ScriptType::Lua;

bool ScriptSystem::Initialize(const ScriptType &type) {
  switch (type) {
  case ScriptType::Lua:
    script = Script::CreateLua();
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
local texture = sn.Texture()
local draw2d = sn.Draw2D(texture)
local font = sn.Font()
font:Load(96)
function Update()
  font:RenderText(texture, "NO DATA", sn.Color(1, 1, 1, 1))
  draw2d.scale = texture:Size()
end
function Draw()
  sn.Graphics.Draw2D(draw2d)
end
)";

void ScriptSystem::RunScene(std::string_view sceneName) {
  if (script) {
    std::string source;
    switch (ScriptSystem::type) {
    case ScriptType::Lua: {
      source = AssetIO::OpenAsString(AssetType::Script,
                                     std::string(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
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