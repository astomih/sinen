#ifndef SINEN_VK_BUFFER_HPP
#define SINEN_VK_BUFFER_HPP
#include "vma.hpp"
namespace sinen {
struct vk_buffer {
  VkBuffer buffer;
  VmaAllocation allocation;
};
} // namespace sinen
#endif // !SINEN_VK_BUFFER_HPP
