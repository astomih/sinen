#ifndef SINEN_SHADER_STAGE_HPP
#define SINEN_SHADER_STAGE_HPP
#include <core/def/types.hpp>
namespace sinen {
enum class ShaderStage : UInt32 {
  Vertex = 1 << 0,
  Fragment = 1 << 1,
  Compute = 1 << 2,
  RayGeneration = 1 << 3,
  AnyHit = 1 << 4,
  ClosestHit = 1 << 5,
  Miss = 1 << 6,
  Intersection = 1 << 7,
  Callable = 1 << 8,
};
}
#endif
