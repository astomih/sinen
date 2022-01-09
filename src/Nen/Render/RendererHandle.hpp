#pragma once
#include <Render/Renderer.hpp>

namespace nen {
class renderer_handle {
public:
  static void SetRenderer(std::shared_ptr<renderer> renderer) {
    mRenderer = renderer;
  }

  static std::shared_ptr<renderer> GetRenderer() { return mRenderer; }

private:
  renderer_handle() = default;
  static std::shared_ptr<renderer> mRenderer;
};
} // namespace nen