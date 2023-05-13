#ifndef SINEN_VK_IMAGE_HPP
#define SINEN_VK_IMAGE_HPP
#include "vma.hpp"
namespace sinen {
struct vk_image {
public:
  VkImage image;
  VmaAllocation allocation;
  VkImageView view;
};
} // namespace sinen
#endif // !SINEN_VK_IMAGE_HP
