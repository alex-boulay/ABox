#include "PreProcUtils.hpp"
#include <iostream>
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

  // Move constructor
  DebugHandler(
      DebugHandler &&other
  ) noexcept
      : debugMessenger(other.debugMessenger)
      , instance(other.instance)
  {
    std::cout << "DebugHandler Move constructor " << std::endl;
    other.instance       = VK_NULL_HANDLE;
    other.debugMessenger = {};
  }
  // Move assignment operator
  DebugHandler &operator=(
      DebugHandler &&other
  ) noexcept
  {
    if (this != &other) {
      this->instance       = other.instance;
      this->debugMessenger = other.debugMessenger;
      other.instance       = VK_NULL_HANDLE;
      other.debugMessenger = {};
    }

    return *this;
  }

  DELETE_COPY(DebugHandler);

  void                               setupDebugMessenger();
  VkDebugUtilsMessengerCreateInfoEXT populateDebugMessenger();

  const VkDebugUtilsMessengerEXT &getDebugMessenger() const
  {
    return debugMessenger;
  }
};
