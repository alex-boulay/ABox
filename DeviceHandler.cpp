#include "DeviceHandler.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <iostream>

DeviceHandler::DeviceHandler(std::unique_ptr<VkInstance> instance):instance(std::move(instance)){
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
  phyDevices.resize(deviceCount);
  vkEnumeratePhysicalDevices(*instance, &deviceCount,phyDevices.data());
}

VkResult DeviceHandler::listPhysicalDevices(){
  std::cout<< "instance : boolean "<< (instance != nullptr) << std::endl;
  
  return VK_SUCCESS;
}

/**
  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      physical.push_back(device);
    }
  }
  if (physicalDevices.back() == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }*/
