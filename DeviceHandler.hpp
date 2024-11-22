#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "vulkan/vulkan.h"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class DeviceBind {
  const VkPhysicalDevice * const physical;
  const VkDevice          device;
public :
  DeviceBind(VkDevice device,const VkPhysicalDevice * const physical):physical(physical),device(device){}
  ~DeviceBind(){ vkDestroyDevice(device,nullptr);}
  
  VkPhysicalDevice getPhysicalDevice()const{ return *physical;}
  VkDevice getDevice()const {return device;}

  DeviceBind(const DeviceBind&)=delete;
  DeviceBind& operator=(const DeviceBind) = delete;
};

/** @brief map a physical device to logical ones
*/
class DeviceHandler{
  std::unique_ptr<VkInstance>   instance;
  std::vector<VkPhysicalDevice> phyDevices;
  std::vector<DeviceBind>       devices;

public:
  VkResult listPhysicalDevices();
  VkResult pickPhysical();
  VkResult addLogicalDevice();
  VkResult clear();

  DeviceHandler();
  DeviceHandler(std::unique_ptr<VkInstance> instance);

  ~DeviceHandler();
  // No copy
  DeviceHandler(const DeviceHandler&)            = delete;
  DeviceHandler& operator=(const DeviceHandler&) = delete;
  // Move possible but as vector are defined no use to move
  DeviceHandler(DeviceHandler&& other)            noexcept = default;
  DeviceHandler& operator=(DeviceHandler&& other) noexcept = default;
};

#endif
