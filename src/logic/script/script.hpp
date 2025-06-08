#ifndef SINEN_SCRIPT
#define SINEN_SCRIPT
#include <memory>
#include <string_view>

namespace sinen {
class IScript {
public:
  virtual ~IScript() = default;
  virtual bool Initialize() = 0;
  virtual void Finalize() = 0;

  virtual void RunScene(std::string_view source) = 0;

  virtual void Update() = 0;
  virtual void Draw() = 0;
};

class Script {
public:
  static std::unique_ptr<IScript> CreateLua();
  static std::unique_ptr<IScript> CreatePython();
};

} // namespace sinen
#endif