#pragma once 

#include <vector>
#include <vulkan/vulkan.h>
#include "DeviceHandler.hpp"

class RessourcesManger{
  std::vector<DeviceHandler> devices;
  VkInstance instance;
  
  RessourcesManger(){
    VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "ABoxApp",
      .applicationVersion = 10000,
      .pEngineName        = "ABox",
      .engineVersion      = 10000,
      .apiVersion         = VK_API_VERSION_1_3
    };

  }
  void displayPhysicalDevices();

};
