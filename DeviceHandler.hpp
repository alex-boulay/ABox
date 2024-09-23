#include "vulkan/vulkan.h"
#include <vector>

/** @brief map a physical device to logical ones
*/
class DeviceHandler{
  const VkPhysicalDevice physical;
  std::vector<VkDevice> devices;

  void listPhysical();
  VkResult pickPhysical();
  VkResult addLogicalDevice();
  VkResult clear();

  DeviceHandler():
};
