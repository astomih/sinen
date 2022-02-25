#include <Nen.hpp>

namespace nen {
handle_t texture_system::create_texture() {
  handle_t handle = texture_handler.create();
  return handle;
}

texture &texture_system::get_texture(handle_t handle) {
  return texture_handler.get(handle);
}

void texture_system::remove_texture(handle_t handle) {
  texture_handler.remove(handle);
}
} // namespace nen