#include <vector>
#include <vulkan/vulkan_core.h>

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
class DebugHandler {

  VkDebugUtilsMessengerEXT debugMessenger;

  bool checkValidationLayerSupport();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  VkDebugUtilsMessengerCreateInfoEXT populateDebugMessenger();
  void                               setupDebugMessenger(VkInstance instance);

   public:
  const VkDebugUtilsMessengerEXT &getDebugMessenger() const
  {
    return debugMessenger;
  }
};
