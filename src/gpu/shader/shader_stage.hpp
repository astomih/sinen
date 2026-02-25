#ifndef SINEN_SHADER_STAGE_HPP
#define SINEN_SHADER_STAGE_HPP
#include <core/def/types.hpp>
namespace sinen {
enum class ShaderStage : UInt32 {
  Vertex = 1 << 0,
  Fragment = 1 << 1,
  Compute = 1 << 2,
};
}
#endif