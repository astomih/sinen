#pragma once
#include "../Math/Vector2.hpp"
#include "../Shader/Shader.hpp"
#include "../Texture/Texture.hpp"
#include "../Utility/handler.hpp"
#include "ObjectType.hpp"

namespace nen {
class draw_object {
public:
  draw_object() = default;
  ~draw_object() = default;
  shader_parameter param;
  std::string textureIndex;
  handle_t texture_handle;
  std::string vertexIndex;
  shader shader_data;
  int drawOrder = 100;
  int nodeNum = 1;
};

} // namespace nen
