#ifndef NEN_TEXTURE_SYSTEM_HPP
#define NEN_TEXTURE_SYSTEM_HPP
#include "../Utility/handler.hpp"
#include "Texture.hpp"
namespace nen {
class texture_system {
public:
  texture_system(class manager &_manager) : m_manager(_manager) {}
  ~texture_system() = default;

  handle_t add_texture(std::string_view file_name);
  handle_t add_texture();
  texture &get_texture(handle_t handle);

  void remove_texture(handle_t handle);

private:
  class manager &m_manager;
  handler<texture> texture_handler;
};
} // namespace nen
#endif