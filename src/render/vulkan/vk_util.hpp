#ifndef SINEN_VK_UTIL_HPP
#define SINEN_VK_UTIL_HPP
#include <SDL3/SDL.h>
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
  vkutil::vk_check_result_code(code, FILE_PREFIX msg)

namespace sinen {
class vkutil {
public:
  template <class U>
  static void destroy_vulkan_object(
      const VkDevice &device, U &object,
      std::function<void(VkDevice, U, VkAllocationCallbacks *)> function) {
    if (object != VK_NULL_HANDLE) {
      function(device, object, nullptr);
      object = VK_NULL_HANDLE;
    }
  }

  class vulkan_exception : public std::runtime_error {
  public:
    vulkan_exception(const std::string &msg)
        : std::runtime_error(msg.c_str()) {}
  };

  static inline void vk_check_result_code(VkResult code,
                                          const std::string &errorMsg) {
    if (code != VK_SUCCESS) {
      throw vulkan_exception(errorMsg);
    }
  }

  static inline VkComponentMapping default_component_mapping() {
    return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                              VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
  }
  static std::string result_to_string(VkResult code);
};
} // namespace sinen

#endif
