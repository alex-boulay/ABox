#include "DeviceHandler.hpp"
#include <vulkan/vulkan_core.h>
#include <system_error>


DeviceHandler::DeviceHandler(const VkInstance& instance):instance(instance){
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if(!deviceCount){
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  pDevices.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, pDevices.data());
}

VkResult DeviceHandler::listPhysicalDevices(const VkInstance& instance){
  /**
  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      physical.push_back(device);
    }
  }
  if (physicalDevices.back() == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }*/
  return VK_SUCCESS;
}
