#pragma once 

#include <vector>
#include <vulkan/vulkan.h>
#include "DeviceHandler.hpp"

class RessourcesManger{
  std::vector<DeviceHandler> devices;
  VkInstance instance;
  
  RessourcesManger(){
    VkApllicationInfo appInfo{
      .sType = VK_STRUCTURE_APPLICATION_INFO,
      .pNext = nullptr,
      .flags = 0,

    }
  }
  void displayPhysicalDevices();

};
