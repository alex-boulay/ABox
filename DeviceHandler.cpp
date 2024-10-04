#include "DeviceHandler.hpp"
#include <vulkan/vulkan_core.h>
#include <system_error>

VkResult DeviceHandler::listPhysicalDevices(const VkInstance& instance){
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if(!deviceCount)
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      physical.push_back(device);
    }
  }
  if (physicalDevices.back() == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  return VK_SUCCESS;
}
