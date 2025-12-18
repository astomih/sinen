#ifndef SINEN_SCRIPT_SYSTEM
#define SINEN_SCRIPT_SYSTEM
#include <memory>
#include <string_view>

namespace sinen {
class IScriptBackend {
public:
  virtual ~IScriptBackend() = default;
  virtual bool initialize() = 0;
  virtual void finalize() = 0;

  virtual void runScene(std::string_view source, std::string_view chunk) = 0;

  virtual void update() = 0;
  virtual void draw() = 0;
};

class ScriptBackend {
public:
  static std::unique_ptr<IScriptBackend> createLua();
};

} // namespace sinen
#endif // !SINEN_SCRIPT_SYSTEM