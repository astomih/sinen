#ifndef SINEN_SCRIPT_SYSTEM
#define SINEN_SCRIPT_SYSTEM
#include <memory>
#include <string_view>

namespace sinen {
class IScriptBackend {
public:
  virtual ~IScriptBackend() = default;
  virtual bool Initialize() = 0;
  virtual void Finalize() = 0;

  virtual void RunScene(std::string_view source) = 0;

  virtual void Update() = 0;
  virtual void Draw() = 0;
};

class ScriptBackend {
public:
  static std::unique_ptr<IScriptBackend> CreateLua();
};

} // namespace sinen
#endif // !SINEN_SCRIPT_SYSTEM