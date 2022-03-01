#pragma once
#include "../Color/Color.hpp"
#include "../Math/Matrix4.hpp"
#include "../Utility/handle_t.hpp"
#include "../Vertex/Vertex.hpp"
#include <memory>
#include <string>
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
  bool Load(std::string_view fileName);
  bool LoadFromMemory(std::vector<char> &buffer, std::string_view ID);

  void fill_color(const color &color);

  const int GetWidth();
  const int GetHeight();
  handle_t handle;
};
} // namespace nen
