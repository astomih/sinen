#include <Nen.hpp>

namespace nen {
handle_t texture_system::add_texture(std::string_view file_name) {
  handle_t handle = texture_handler.add(file_name);
  return handle;
}
handle_t texture_system::add_texture() {
  handle_t handle = texture_handler.add();
  return handle;
}

texture &texture_system::get_texture(handle_t handle) {
  return texture_handler.get(handle);
}

void texture_system::remove_texture(handle_t handle) {
  texture_handler.remove(handle);
}
} // namespace nen