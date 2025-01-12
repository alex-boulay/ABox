#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "PreProcUtils.hpp"
#include "SwapchainManager.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ABox_Utils {

/**
 * @struct QueueFamilyIndices
 * @brief represent optional queue family indices
 */
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicQueueIndex;
  std::optional<uint32_t> presentQueueIndex;
};
/**
 * @struct DeviceBoundElements
 * @brief Represents elements bounded to the Logical Device
 */
struct DeviceBoundElements {
  VkPhysicalDevice                physical;
  QueueFamilyIndices              fIndices;
  std::optional<SwapchainManager> swapchain;
};

/**
 * @class DeviceHandler
 * @brief Handle specifics to logical devices and their different bindings
 */
class DeviceHandler {
  std::vector<VkPhysicalDevice>                          phyDevices;
  std::vector<VkDevice>                                  devices;
  std::unordered_map<uint_fast16_t, DeviceBoundElements> deviceMap;

  std::set<uint32_t>    getQueueFamilyIndices(QueueFamilyIndices fi);
  std::vector<uint32_t> listQueueFamilyIndices(QueueFamilyIndices fi);

  bool destroyed = false;

   public:
  DeviceHandler(
      DeviceHandler &&other
  ) noexcept
      : phyDevices(other.phyDevices)
      , devices(other.devices)
  {
    for (auto &a : other.deviceMap) {
      deviceMap[a.first] = {
          .physical  = a.second.physical,
          .fIndices  = a.second.fIndices,
          .swapchain = std::move(a.second.swapchain)
      };
      a.second.swapchain = SwapchainManager();
    }
  }
  DeviceHandler &operator=(
      DeviceHandler &&other
  ) noexcept
  {
    if (this != &other) {
      // Copy assignment: copy the members from `other` to `this`
      phyDevices = other.phyDevices;
      devices    = other.devices;

      other.phyDevices.clear();
      other.devices.clear();
      for (auto &a : other.deviceMap) {
        deviceMap[a.first] = {
            .physical  = a.second.physical,
            .fIndices  = a.second.fIndices,
            .swapchain = std::move(a.second.swapchain)
        };

        a.second.swapchain = SwapchainManager();
      }
    }
    return *this;
  }

  VkResult listPhysicalDevices() const;

  /**
   * @brief add a Logical device while guessing which Physical Device is the
   * best suited to do the job
   *
   * @param VkSurfaceKHR surface the surface which will bind presentation
   * @return VkResult : VK_SUCCESS if succeded else a corresponding error
   * value
   */
  VkResult addLogicalDevice(VkSurfaceKHR surface);
  /**
   *
   * @param uint32_t index the physical device index to select
   * @param[[VkSurfaceKHR] surface the surface which will bind presentation
   * @return a VkResult if VK_SUCCESS if succeded else a corresponding error
   * value
   */
  VkResult addLogicalDevice(uint32_t index, VkSurfaceKHR surface);

  // TODO : VkResult clear();
  uint32_t listQueueFamilies();
  VkResult DeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices
      loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);

  uint32_t findBestPhysicalDevice();

  DeviceHandler() {};
  DeviceHandler(VkInstance instance);

  void removeBindings();

  VkDevice *getDevice(uint32_t index);
  // DeviceBoundElements getBoundElements(uint_fast16_t devIndex) const;
  VkResult  addSwapchain(
       uint32_t                        width,
       uint32_t                        height,
       std::function<VkSurfaceKHR *()> getSurface,
       uint_fast8_t                    devIndex
   );

  // No copy
  DELETE_COPY(
      DeviceHandler
  )
};

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag);
OSTREAM_OP(const VkQueueFamilyProperties &prop);
OSTREAM_OP(const VkExtent3D &ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
} // namespace ABox_Utils
#endif
