#ifndef NEN_FONT_SYSTEM_HPP
#define NEN_FONT_SYSTEM_HPP
#include "../Utility/handler.hpp"
#include "Font.hpp"
namespace nen {
class font_system {
public:
  font_system(class manager &_manager) : m_manager(_manager) {}
  ~font_system() = default;

  handle_t add_font(std::string_view file_name, int32_t size);
  handle_t create_font();
  font &get_font(handle_t handle);

  void remove_font(handle_t handle);

private:
  class manager &m_manager;
  handler<font> font_handler;
};
} // namespace nen
#endif