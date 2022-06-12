#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <draw_object/draw_object.hpp>
#include <vk_mem_alloc.h>
namespace nen {
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
class vk_draw_object {
public:
  std::vector<VkDescriptorSet> descripterSet;
  std::vector<vk_buffer_object> uniformBuffers;
  bool isInstance = false;
  std::shared_ptr<nen::draw_object> drawObject;
};
} // namespace nen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)