#pragma once 

#include <vector>
#include <vulkan/vulkan.h>

class RessourcesManger{
  std::vector<VkDevice> devices;
  VkInstance instance;
  

  void displayPhysicalDevices();

};
