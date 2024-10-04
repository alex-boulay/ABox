#include "vulkan/vulkan.h"
#include <vector>
#include <vulkan/vulkan_core.h>

struct deviceBind {
  const VkPhysicalDevice * physical;
  const VkDevice           device  ;
};

/** @brief map a physical device to logical ones
*/
class DeviceHandler{
  const std::vector<VkPhysicalDevice> physicalList;
  std::vector<deviceBind> devices;

  VkResult listPhysicalDevices();
  VkResult pickPhysical();
  VkResult addLogicalDevice();
  VkResult clear();

  DeviceHandler();
};
