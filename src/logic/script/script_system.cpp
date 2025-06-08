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
static const char *nothingScenePython = R"(
from sinen import *
texture = Texture()
draw2d = Draw2D()
draw2d.material.append(texture)
font = Font()
font.load(96)

def update():
  font.render_text(texture, "NO DATA", Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
)";

static const char *nothingSceneLua = R"(
local texture = Texture()
local draw2d = Draw2D()
draw2d.material:append(texture)
local font = Font()
font:load(96)
function update()
  font:render_text(texture, "NO DATA", Color(1, 1, 1, 1))
  draw2d.scale = texture:size()
end
function draw()
  draw2d:draw()
end
)";

void ScriptSystem::RunScene(std::string_view sceneName) {
  if (script) {
    std::string source;
    switch (ScriptSystem::type) {
    case ScriptType::Lua: {
      source = DataStream::open_as_string(AssetType::Script,
                                          std::string(sceneName) + ".lua");
      if (source.empty()) {
        source = nothingSceneLua;
      }
    } break;
    case ScriptType::Python: {
      source = DataStream::open_as_string(AssetType::Script,
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