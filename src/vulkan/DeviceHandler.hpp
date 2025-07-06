#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "CommandsHandler.hpp"
#include "GraphicsPipeline.hpp"
#include "PreProcUtils.hpp"
#include "ShaderHandler.hpp"
#include "SwapchainManager.hpp"
#include "SynchronisationManager.hpp"
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ABox_Utils {

DEFINE_VK_MEMORY_WRAPPER_SOLO(
    VkDevice,
    Device,
    vkDestroyDevice
)

/**
 * @struct QueueFamilyIndices
 * @brief represent optional queue family indices
 */
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsQueueIndex;
  std::optional<uint32_t> presentQueueIndex;
};

/**
 * @struct DeviceBoundElements
 * @brief Represents elements bounded to the Logical Device
 */
class DeviceBoundElements {
  DeviceWrapper      device;
  VkPhysicalDevice   physical;
  QueueFamilyIndices fIndices;
  FrameSyncArray     syncM;
  CommandsHandler    commands;

   public:
  VkQueue graphicsQueue = VK_NULL_HANDLE; // move to Queue Management ??
  VkQueue presentQueue  = VK_NULL_HANDLE;

  std::optional<SwapchainManager>                      swapchain;
  std::optional<GraphicsPipeline>                      graphicsppl;
  std::unordered_map<std::string, ShaderModuleWrapper> loadedShaders;

  DeviceBoundElements(
      VkDevice           logDevice,
      VkPhysicalDevice   phyDev,
      QueueFamilyIndices familyIndices
  )
      : device(logDevice)
      , physical(phyDev)
      , fIndices(familyIndices)
      , syncM(logDevice) // TODO size )
  {
  }
  DELETE_MOVE(DeviceBoundElements);
  DELETE_COPY(DeviceBoundElements);
  ~DeviceBoundElements() = default;

  const DeviceWrapper &getDevice() const { return device; }

  DeviceWrapper     *getDevicePtr() { return &device; }
  VkPhysicalDevice   getPhysicalDevice() { return physical; }
  QueueFamilyIndices getFamilyQueueIndices() { return fIndices; }
  FrameSyncArray    *getFrameSyncArray() { return &syncM; }
};

/**
 * @class DeviceHandler
 * @brief Handle specifics to logical devices and their different bindings
 */
class DeviceHandler {
  std::vector<VkPhysicalDevice>                phyDevices;
  std::list<DeviceBoundElements>               devices;
  std::map<std::string, DeviceBoundElements *> deviceNames;

  std::set<uint32_t>    getQueueFamilyIndices(QueueFamilyIndices fi);
  std::vector<uint32_t> listQueueFamilyIndices(QueueFamilyIndices fi);

   public:
  DELETE_COPY(DeviceHandler);
  DELETE_MOVE(DeviceHandler);

  ~DeviceHandler() = default;

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
  VkResult
      addLogicalDevice(uint32_t index, VkSurfaceKHR surface, std::string name);

  // TODO : VkResult clear();
  uint32_t listQueueFamilies();
  VkResult DeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices
      loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);

  uint32_t findBestPhysicalDevice();

  DeviceHandler() {};
  DeviceHandler(VkInstance instance);

  DeviceBoundElements *getDBE(uint32_t index);
  DeviceBoundElements *getDBE(std::string name);
  VkDevice             getDevice(uint32_t index);

  inline bool hasDevice(
      uint32_t index
  ) const
  {
    return index < devices.size();
  }

  // DeviceBoundElements getBoundElements(uint_fast16_t devIndex) const;
  VkResult addSwapchain(
      uint32_t      width,
      uint32_t      height,
      VkSurfaceKHR *surface,
      uint_fast8_t  devIndex
  );

  std::pair<VkResult, VkShaderModule>
      loadShader(uint_fast16_t deviceIndex, const ShaderDataFile &sdf);

  /**
   * @brief add a GraphicsPipeline to a LogicalDevice which must have a
   * swapchain
   */
  VkResult addGraphicsPipeline(
      uint32_t                         deviceIndex,
      const std::list<ShaderDataFile> &shaderFiles
  );

  VkResult createFramebuffers(
      uint32_t deviceIndex = 0u
  )
  {
    if (devices.size() > deviceIndex) {
      DeviceBoundElements *dbe = getDBE(deviceIndex);
      if (dbe->swapchain.has_value() && dbe->graphicsppl.has_value()) {
        return dbe->swapchain.value().createFramebuffers(
            dbe->graphicsppl.value().getRenderPass(),
            dbe->getDevice()
        );
      }
      else if (!dbe->swapchain.has_value()) {
        throw std::runtime_error(
            "No Swapchain make Framebuffer creation impossible !"
        );
      }
      else {
        throw std::runtime_error(
            "No Graphics Pipeline makes Framebuffer creation impossible !"
        );
      }
    }
    else {
      throw std::runtime_error("No Device present at given Index !");
    }
    return VK_SUCCESS;
  }
};

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag);
OSTREAM_OP(const VkQueueFamilyProperties &prop);
OSTREAM_OP(const VkExtent3D &ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
} // namespace ABox_Utils
#endif
