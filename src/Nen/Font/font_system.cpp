#include <Nen.hpp>
namespace nen {
handle_t font_system::add_font(std::string_view file_name, int32_t size) {
  handle_t handle = font_handler.add(file_name, size);
  return handle;
}
handle_t font_system::add_font() {
  handle_t handle = font_handler.add();
  return handle;
}

font &font_system::get_font(handle_t handle) {
  return font_handler.get(handle);
}

void font_system::remove_font(handle_t handle) { font_handler.remove(handle); }

} // namespace nen