#ifndef SINEN_INSTANCING_HPP
#define SINEN_INSTANCING_HPP

#include "../drawable/drawable.hpp"
#include "../drawable/object_type.hpp"
#include "../texture/texture.hpp"
#include "instance_data.hpp"
#include <cstddef>
#include <memory>
#include <vector>
namespace sinen {

class instancing {
public:
  instancing() = default;
  instancing(instancing &&) = default;
  instancing(const instancing &) = default;
  instancing &operator=(instancing &&) = default;
  instancing &operator=(const instancing &) = default;
  ~instancing() = default;
  void world_to_instance_data(const matrix4 &mat, instance_data &data);

  std::shared_ptr<drawable> object;
  object_type type;
  std::size_t size;
  std::vector<instance_data> data;

private:
};

} // namespace sinen
#endif // !SINEN_INSTANCING_HPP
