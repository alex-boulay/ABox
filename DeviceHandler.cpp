#include "DeviceHandler.hpp"
#include <iostream>
#include <ostream>
#include <vulkan/vulkan_core.h>

#define OSTREAM_OP(X) std::ostream &operator<<(std::ostream &os, X)

OSTREAM_OP(const VkPhysicalDeviceType &phyT) {
  switch (phyT) {
  case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    return os << "other type GPU";
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return os << "integrated GPU";
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return os << "discrete GPU";
  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    return os << "virtual GPU";
  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return os << "CPU";
  default:
    return os << " undefined GPU type";
  }
}

OSTREAM_OP(const VkPhysicalDeviceProperties &phyP) {
  os << "------- Physical Device Properties ----------\n";
  os << "\t API version : " << phyP.apiVersion << '\n';
  os << "\t driver version : " << phyP.driverVersion << '\n';
  os << "\t vendor ID : " << phyP.vendorID << '\n';
  os << "\t device ID : " << phyP.deviceID << '\n';
  os << "\t device type : " << phyP.deviceType << '\n';
  os << "\t device name : " << phyP.deviceName << '\n';
  return os;
}

DeviceHandler::~DeviceHandler() noexcept {
  for (auto a : devices)
    vkDestroyDevice(a, nullptr);
  std::vector<VkDevice>().swap(devices);
  std::vector<VkPhysicalDevice>().swap(phyDevices);
  std::unordered_map<VkDevice, VkPhysicalDevice>().swap(deviceMap);
}

DeviceHandler::DeviceHandler(VkInstance instance) : instance(instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  std::cout << "Total number of phy devices : " << deviceCount << '\n';
  phyDevices.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, phyDevices.data());
}

VkResult DeviceHandler::listPhysicalDevices() const {
  uint32_t index = 0;
  for (auto physical : phyDevices) {
    VkPhysicalDeviceProperties phyProp = {};
    vkGetPhysicalDeviceProperties(physical, &phyProp);
    std::cout << "Device n°" << index << " : \n" << phyProp;
    index++;
  }
  return index > 0 ? VK_SUCCESS : VK_ERROR_DEVICE_LOST;
}

VkResult DeviceHandler::addLogicalDevice(uint32_t index) {
  std::cout << "1" << std::endl;
  VkDeviceCreateInfo devInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0u,
      .queueCreateInfoCount = 0u,
      .pQueueCreateInfos = 0u,
      .enabledLayerCount = 0u,
      .ppEnabledLayerNames = 0u,
      .enabledExtensionCount = 0u,
      .ppEnabledExtensionNames = 0u,
      .pEnabledFeatures = 0u,
  };
  std::cout << "2" << std::endl;
  devices.emplace_back(VkDevice{});
  std::cout << "3" << std::endl;
  VkResult res =
      vkCreateDevice(phyDevices.at(index), &devInfo, nullptr, &devices.back());
  std::cout << "4" << std::endl;
  if (res == VK_SUCCESS)
    deviceMap[devices.back()] = phyDevices.at(index);
  else
    devices.pop_back();
  std::cout << "5" << std::endl;
  return res;
}
