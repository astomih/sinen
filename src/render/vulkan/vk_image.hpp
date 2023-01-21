#ifndef SINEN_VK_IMAGE_HPP
#define SINEN_VK_IMAGE_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include <vk_mem_alloc.h>
namespace sinen {
struct vk_image {
public:
  VkImage image;
  VmaAllocation allocation;
  VkImageView view;
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(MOBILE)
#endif // !SINEN_VK_IMAGE_HP
