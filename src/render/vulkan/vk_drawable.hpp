#ifndef SINEN_VK_DRAWABLE_HPP
#define SINEN_VK_DRAWABLE_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include "vk_buffer.hpp"
#include <drawable/drawable.hpp>

namespace sinen {
class vk_drawable {
public:
  std::array<VkDescriptorSet, 2> descriptor_sets;
  std::array<VkDescriptorSet, 2> descriptor_sets_for_depth;
  std::array<vk_buffer, 2> uniformBuffers;
  std::shared_ptr<drawable> drawable_obj;
  vk_buffer instance_buffer;
  drawable::parameter get_parameter() { return drawable_obj->param; }
  shader get_shader() { return drawable_obj->shade; }
  texture get_texture() { return drawable_obj->binding_texture; }
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)
#endif // !SINEN_VK_DRAWABLE_HPP
