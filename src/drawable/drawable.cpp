#include <drawable/drawable.hpp>

namespace sinen {
drawable::drawable() {
  float f[16] = {};
  this->param.user = matrix4(f);
}
drawable::~drawable() {}
} // namespace sinen
