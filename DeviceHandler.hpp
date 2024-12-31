#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "PreProcUtils.hpp"
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>
/** @brief map a physical device to logical ones
 */

namespace ABox_Utils {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicQueueIndex;
  std::optional<uint32_t> renderQueueIndex;
};

class DeviceHandler {
  VkInstance                                     instance;
  std::vector<VkPhysicalDevice>                  phyDevices;
  std::vector<VkDevice>                          devices;
  std::unordered_map<VkDevice, VkPhysicalDevice> deviceMap;
  QueueFamilyIndices                             fIndices;

   public:
  VkResult listPhysicalDevices() const;

  VkResult addLogicalDevice(VkSurfaceKHR surface);
  VkResult addLogicalDevice(uint32_t index, VkSurfaceKHR surface);

  VkResult clear();
  uint32_t listQueueFamilies();
  VkResult DeviceExtensionSupport(VkPhysicalDevice device);
  void     loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);

  DeviceHandler() {};
  DeviceHandler(VkInstance instance);

  ~DeviceHandler() noexcept;
  // No copy
  DeviceHandler(const DeviceHandler &)                     = delete;
  DeviceHandler &operator=(const DeviceHandler &)          = delete;
  // Move possible but as vector are defined no use to move
  DeviceHandler(DeviceHandler &&other) noexcept            = default;
  DeviceHandler &operator=(DeviceHandler &&other) noexcept = default;
};

std::stringstream vkQueueFlagSS(VkQueueFlags flag);
OSTREAM_OP(VkQueueFamilyProperties prop);
OSTREAM_OP(VkExtent3D ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
} // namespace ABox_Utils
#endif
