#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "vulkan/vulkan_core.h"
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>
/** @brief map a physical device to logical ones
 */

#ifdef DEBUG_VK_ABOX
#define VK_ABOX_VALDIATION_LAYERS
#endif

namespace ABox_Utils {

class DeviceHandler {
  VkInstance instance;
  std::vector<VkPhysicalDevice> phyDevices;
  std::vector<VkDevice> devices;
  std::unordered_map<VkDevice, VkPhysicalDevice> deviceMap;

public:
  VkResult listPhysicalDevices() const;
  // VkResult pickPhysical(uint32_t index);
  VkResult addLogicalDevice(uint32_t index);
  VkResult clear();

  DeviceHandler() {};
  DeviceHandler(VkInstance instance);

  ~DeviceHandler() noexcept;
  // No copy
  DeviceHandler(const DeviceHandler &) = delete;
  DeviceHandler &operator=(const DeviceHandler &) = delete;
  // Move possible but as vector are defined no use to move
  DeviceHandler(DeviceHandler &&other) noexcept = default;
  DeviceHandler &operator=(DeviceHandler &&other) noexcept = default;
};
} // namespace ABox_Utils
#endif
