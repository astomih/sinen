#pragma once
#include "../DrawObject/DrawObject.hpp"
#include "../Texture/Texture.hpp"
#include "DrawObject/ObjectType.hpp"
#include "instance_data.hpp"
#include <cstddef>
#include <memory>
#include <vector>
namespace nen {

class instancing {
public:
  instancing() = default;
  instancing(instancing &&) = default;
  instancing(const instancing &) = default;
  instancing &operator=(instancing &&) = default;
  instancing &operator=(const instancing &) = default;
  ~instancing() = default;
  void world_to_instance_data(const matrix4 &mat, instance_data &data);

  std::shared_ptr<draw_object> object;
  object_type type;
  std::size_t size;
  std::vector<instance_data> data;

private:
};

} // namespace nen