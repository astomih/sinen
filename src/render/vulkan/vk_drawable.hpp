#ifndef SINEN_VK_DRAWABLE_HPP
#define SINEN_VK_DRAWABLE_HPP
#include "vk_buffer.hpp"
#include <array>
#include <drawable/drawable.hpp>

namespace sinen {
class vk_drawable {
public:
  std::array<VkDescriptorSet, 2> descriptor_sets;
  std::array<VkDescriptorSet, 2> descriptor_sets_for_depth;
  std::array<vk_buffer, 2> uniformBuffers;
  std::shared_ptr<Drawable> drawable_obj;
  vk_buffer instance_buffer;
  Drawable::parameter get_parameter() { return drawable_obj->param; }
  Shader get_shader() { return drawable_obj->shade; }
  Texture get_texture() { return drawable_obj->binding_texture; }
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)
