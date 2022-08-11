#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include "../color/color.hpp"
#include "../math/matrix4.hpp"
#include "../utility/handle_t.hpp"
#include "../vertex/vertex.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sinen {
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
  bool load_from_memory(std::vector<char> &buffer, std::string_view ID);

  texture copy();

  void fill_color(const color &color);
  void blend_color(const color &color);

  vector2 size();

  handle_t handle;
  std::shared_ptr<bool> is_need_update;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
