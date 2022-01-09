#pragma once
#include <functional>
#include <memory>
#include <string>

namespace sol {
class state;
}

namespace nen {
class script {
public:
  script() = default;
  static void Create();
  static class sol::state *GetSolState();
  static const script &Get() { return *instance; }
  static void DoScript(std::string_view fileName);

private:
  static inline std::unique_ptr<script> instance;
};
} // namespace nen
