#include <vulkan/vulkan.h>

class DebugHandler {

  VkDebugUtilsMessengerEXT debugMessenger = {};
  VkInstance               instance       = VK_NULL_HANDLE;

  bool checkValidationLayerSupport();
  bool checkDeviceExtensionSupport();

   public:
  DebugHandler() {};
  DebugHandler(VkInstance instance);
  ~DebugHandler();

  void                               setupDebugMessenger();
  VkDebugUtilsMessengerCreateInfoEXT populateDebugMessenger();

  const VkDebugUtilsMessengerEXT &getDebugMessenger() const
  {
    return debugMessenger;
  }
};
