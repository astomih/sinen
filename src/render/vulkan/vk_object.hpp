#ifndef SINEN_VK_OBJECT_HPP
#define SINEN_VK_OBJECT_HPP
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <drawable/drawable.hpp>
#include <vk_mem_alloc.h>
namespace sinen {
class vk_buffer_object {
public:
  VkBuffer buffer;
  VmaAllocation allocation;
};
struct vk_image_object {
public:
  VkImage image;
  VmaAllocation allocation;
  VkImageView view;
};
class vk_drawable {
public:
  std::vector<VkDescriptorSet> descriptor_set;
  std::vector<vk_buffer_object> uniformBuffers;
  bool isInstance = false;
  std::shared_ptr<drawable> drawable_obj;
  drawable::parameter get_parameter() { return drawable_obj->param; }
  shader get_shader() { return drawable_obj->shade; }
  texture get_texture() { return drawable_obj->binding_texture; }
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)
#endif // !SINEN_VK_OBJECT_HPP
