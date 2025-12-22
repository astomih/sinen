#ifndef SINEN_SCRIPT_SYSTEM
#define SINEN_SCRIPT_SYSTEM
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>


namespace sinen {
class IScriptBackend {
public:
  virtual ~IScriptBackend() = default;
  virtual bool initialize() = 0;
  virtual void finalize() = 0;

  virtual void runScene(StringView source, StringView chunk) = 0;

  virtual void update() = 0;
  virtual void draw() = 0;
};

class ScriptBackend {
public:
  static UniquePtr<IScriptBackend> createLua();
};

} // namespace sinen
#endif // !SINEN_SCRIPT_SYSTEM