// internal
#include "script_system.hpp"
#include "script.hpp"
#include <core/io/data_stream.hpp>

namespace sinen {
std::unique_ptr<IScript> ScriptSystem::script = nullptr;
ScriptType ScriptSystem::type = ScriptType::Python;

bool ScriptSystem::Initialize(const ScriptType &type) {
  switch (type) {
  case ScriptType::Lua:
    script = Script::CreateLua();
    ScriptSystem::type = ScriptType::Lua;
    break;
  case ScriptType::Python:
    script = Script::CreatePython();
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
      source = DataStream::OpenAsString(AssetType::Script,
                                        std::string(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
      }
    } break;
    case ScriptType::Python: {
      source = DataStream::OpenAsString(AssetType::Script,
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