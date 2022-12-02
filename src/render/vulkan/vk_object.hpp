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
  std::vector<VkDescriptorSet> descripterSet;
  std::vector<vk_buffer_object> uniformBuffers;
  bool isInstance = false;
  std::shared_ptr<drawable> p_drawable;
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)
#endif // !SINEN_VK_OBJECT_HPP
