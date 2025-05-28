// internal
#include "script_system.hpp"
#include "script.hpp"
#include <core/io/data_stream.hpp>

namespace sinen {
std::unique_ptr<IScript> ScriptSystem::script = nullptr;
ScriptType ScriptSystem::type = ScriptType::Python;

bool ScriptSystem::Initialize(const ScriptType &type) {
  switch (type) {
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
static const char *nothingScene = R"(
from sinen import *
texture = Texture()
draw2d = Draw2D(texture)
font = Font()
font.load(96)

def update():
  font.render_text(texture, "NO SCENE", Color(1, 1, 1, 1))
  draw2d.scale = texture.size()

def draw():
  draw2d.draw()
)";

void ScriptSystem::RunScene(std::string_view sceneName) {
  if (script) {
    std::string source;
    switch (ScriptSystem::type) {
    case ScriptType::Python: {
      source = DataStream::open_as_string(AssetType::Script,
                                          std::string(sceneName) + ".py");
    } break;
    default:
      break;
    }
    if (source.empty()) {
      source = nothingScene;
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