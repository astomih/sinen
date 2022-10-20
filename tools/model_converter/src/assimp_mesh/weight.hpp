#ifndef SINEN_WEIGHT_HPP
#define SINEN_WEIGHT_HPP
#include <cstdint>
namespace sinen {
struct Weight {
  uint32_t vertex_id;
  float value;
};
} // namespace sinen
#endif // SINEN_WEIGHT_HPP