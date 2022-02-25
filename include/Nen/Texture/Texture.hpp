#pragma once
#include "../Color/Color.hpp"
#include "../Math/Matrix4.hpp"
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
  texture(std::string_view file_name);
  ~texture();
  bool Load(std::string_view fileName);
  bool LoadFromMemory(std::vector<char> &buffer, std::string_view ID);

  void fill_color(const color &color, std::string_view ID);

  const int GetWidth() { return width; }
  const int GetHeight() { return height; }
  void SetWidth(const int w) {
    if (w >= 0)
      this->width = w;
  }
  void SetHeight(const int h) {
    if (h >= 0)
      this->height = h;
  }

  std::string id = "default";

private:
  class Impl;
  std::unique_ptr<Impl> impl;
  int width = 0;
  int height = 0;
};
} // namespace nen
