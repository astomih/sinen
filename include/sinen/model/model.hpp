#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include "../physics/collision.hpp"
#include "../vertex/vertex_array.hpp"
#include <string_view>

namespace sinen {
class model {
public:
  model() = default;
  ~model() = default;

  void load(std::string_view str, std::string_view name);
  aabb m_aabb;

private:
  vertex_array m_array;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
