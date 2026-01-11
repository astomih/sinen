#ifndef SINEN_BUILTIN_PIPELINE
#define SINEN_BUILTIN_PIPELINE
#include "graphics_pipeline.hpp"
namespace sinen {
class BuiltinPipeline {
public:
  static bool initialize();
  static void shutdown();
  static GraphicsPipeline getDefault3D();
  static GraphicsPipeline getInstanced3D();
  static GraphicsPipeline getDefault2D();
  static GraphicsPipeline getCubemap();
};
} // namespace sinen
#endif