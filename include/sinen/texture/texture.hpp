#pragma once
#include "../color/color.hpp"
#include "../math/matrix4.hpp"
#include "../utility/handle_t.hpp"
#include "../vertex/vertex.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace nen {
struct shader_parameter {
  matrix4 world;
  matrix4 view;
  matrix4 proj;
};

class texture {
public:
  texture();
  ~texture();
  bool load(std::string_view fileName);
  bool LoadFromMemory(std::vector<char> &buffer, std::string_view ID);

  void fill_color(const color &color);

  vector2 size();

  handle_t handle;
};
} // namespace nen
