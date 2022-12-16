#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <SDL.h>
#include <vulkan/vulkan.h>

#include <array>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <vector>

#define STRINGFY(s) #s
#define TO_STRING(x) STRINGFY(x)
#define FILE_PREFIX __FILE__ "(" TO_STRING(__LINE__) "): "
#define ThrowIfFailed(code, msg)                                               \
  vkutil::CheckResultCodeVk(code, FILE_PREFIX msg)

namespace sinen {
class vkutil {
public:
  template <class U>
  static void DestroyVulkanObject(
      const VkDevice &device, U &object,
      std::function<void(VkDevice, U, VkAllocationCallbacks *)> function) {
    if (object != VK_NULL_HANDLE) {
      function(device, object, nullptr);
      object = VK_NULL_HANDLE;
    }
  }

  class VulkanException : public std::runtime_error {
  public:
    VulkanException(const std::string &msg) : std::runtime_error(msg.c_str()) {}
  };

  static inline void CheckResultCodeVk(VkResult code,
                                       const std::string &errorMsg) {
    if (code != VK_SUCCESS) {
      throw VulkanException(errorMsg);
    }
  }

  static inline VkAttachmentDescription GetAttachmentDescription(
      VkFormat format, VkImageLayout before, VkImageLayout after,
      VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
    return VkAttachmentDescription{0,
                                   format,
                                   samples,
                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   VK_ATTACHMENT_STORE_OP_STORE,
                                   VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                   VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                   before,
                                   after};
  }

  static inline VkComponentMapping DefaultComponentMapping() {
    return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                              VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
  }
};
} // namespace sinen

#endif
