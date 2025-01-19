#include <vulkan/vulkan.h>

class DebugHandler {

  VkDebugUtilsMessengerEXT debugMessenger;
  VkInstance               instance;

  bool checkValidationLayerSupport();
  bool checkDeviceExtensionSupport();

  void setupDebugMessenger();

   public:
  DebugHandler(VkInstance instance);
  ~DebugHandler();

  VkDebugUtilsMessengerCreateInfoEXT populateDebugMessenger();

  const VkDebugUtilsMessengerEXT &getDebugMessenger() const
  {
    return debugMessenger;
  }
};
