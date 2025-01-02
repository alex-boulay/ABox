#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "PreProcUtils.hpp"
#include <optional>
#include <set>
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
struct DeviceBoundElements {
  VkPhysicalDevice   physical;
  QueueFamilyIndices fIndices;
};

class DeviceHandler {
  VkInstance                                        instance;
  std::vector<VkPhysicalDevice>                     phyDevices;
  std::vector<VkDevice>                             devices;
  std::unordered_map<VkDevice, DeviceBoundElements> deviceMap;

  std::set<uint32_t>    getQueueFamilyIndices(QueueFamilyIndices fi);
  std::vector<uint32_t> listQueueFamilyIndices(QueueFamilyIndices fi);

   public:
  VkResult listPhysicalDevices() const;

  VkResult addLogicalDevice(VkSurfaceKHR surface);
  VkResult addLogicalDevice(uint32_t index, VkSurfaceKHR surface);

  VkResult clear();
  uint32_t listQueueFamilies();
  VkResult DeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices
      loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);

  uint32_t findBestPhysicalDevice();

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

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag);
OSTREAM_OP(const VkQueueFamilyProperties &prop);
OSTREAM_OP(const VkExtent3D &ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
} // namespace ABox_Utils
#endif
