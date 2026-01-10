#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP

#include "CommandsHandler.hpp"
#include "FrameBufferBroker.hpp"
#include "Logger.hpp"
#include "PipelineManager.hpp"
#include "PreProcUtils.hpp"
#include "RenderPassManager.hpp"
#include "ShaderHandler.hpp"
#include "SwapchainManager.hpp"
#include "SynchronisationManager.hpp"
#include <cstdint>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ABox_Utils {

DEFINE_VK_MEMORY_WRAPPER_SOLO(VkDevice, Device, vkDestroyDevice)

/**
 * @struct DeviceBoundElements
 * @brief Represents elements bounded to the Logical Device
 */
class DeviceBoundElements {
  DeviceWrapper            device;
  VkPhysicalDevice         physical;
  const QueueFamilyIndices fIndices;
  FrameSyncArray           syncM;
  CommandsHandler          commands;

   public:
  VkQueue graphicsQueue = VK_NULL_HANDLE; // move to Queue Management ??
  VkQueue presentQueue  = VK_NULL_HANDLE;

  SwapchainPool     swapchains;
  RenderPassManager rpm;
  FrameBufferBroker fbb;
  PipelineManager   pipelineManager;

  DeviceBoundElements(
      VkDevice           logDevice,
      VkPhysicalDevice   phyDev,
      QueueFamilyIndices queueRoleIndices
  )
      : device(logDevice)
      , physical(phyDev)
      , fIndices(queueRoleIndices)
      , syncM(logDevice)
      , commands(device, fIndices)
  {
  }

  DELETE_MOVE(DeviceBoundElements);
  DELETE_COPY(DeviceBoundElements);
  ~DeviceBoundElements() = default;

  const DeviceWrapper &getDevice() const { return device; }

  DeviceWrapper *getDevicePtr() { return &device; }

  VkPhysicalDevice getPhysicalDevice() { return physical; }

  QueueFamilyIndices getFamilyQueueIndices() { return fIndices; }

  FrameSyncArray *getFrameSyncArray() { return &syncM; }

  CommandsHandler *getCommandHandler() { return &commands; }

  VkResult recordCommandBuffer(uint32_t imageIndex, uint32_t commandBufferIndex)
  {
    ABOX_LOG_PER_FRAME << "Recording commands Img " << imageIndex
                       << " commandBufferIndex: " << commandBufferIndex;

    GraphicsPipeline *mainPipeline = pipelineManager.getMainGraphicsPipeline();
    if (!mainPipeline) {
      LOG_ERROR("Pipeline") << "No main graphics pipeline set";
      throw std::runtime_error(
          "Wrong graphics pipeline target during recordcommandbuffer"
      );
    }

    if (swapchains.empty()) {
      LOG_ERROR("Vulkan") << "No Value in swapchain";
      throw std::runtime_error(
          "Wrong swapchain target during recordcommandbuffer"
      );
    }
    return commands.top().recordCommandBuffer(
        *mainPipeline,
        swapchains.front(),
        rpm.front().get(),
        imageIndex,
        commandBufferIndex
    );
  }
};

/**
 * @class DeviceHandler
 * @brief Handle specifics to logical devices and their different bindings
 */
class DeviceHandler {
  std::vector<VkPhysicalDevice>                phyDevices;
  std::list<DeviceBoundElements>               devices;
  std::map<std::string, DeviceBoundElements *> deviceNames;

  std::set<uint32_t> getQueueFamilyIndices(
      QueueFamilyIndices fi
  ); // TODO aim for a loaded device instead

  std::vector<uint32_t> listQueueFamilyIndices(
      QueueFamilyIndices fi
  ); // TODO aim for a loaded device Instead

   public:
  DELETE_COPY(DeviceHandler);
  DELETE_MOVE(DeviceHandler);

  ~DeviceHandler() = default;

  VkResult listPhysicalDevices() const;

  void waitIdle();
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
      addLogicalDevice(VkSurfaceKHR surface, std::string name, uint32_t index);

  // TODO : VkResult clear();
  uint32_t listQueueFamilies();
  // VkResult DeviceExtensionSupport(VkPhysicalDevice device);
  std::unordered_map<QueueRole, uint32_t>
           loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);
  uint32_t findBestPhysicalDevice();

  DeviceHandler() {};
  DeviceHandler(VkInstance instance);

  DeviceBoundElements *getDBE(uint32_t index);
  DeviceBoundElements *getDBE(std::string name);
  VkDevice             getDevice(uint32_t index);

  inline bool hasDevice(uint32_t index) const { return index < devices.size(); }

  // DeviceBoundElements getBoundElements(uint_fast16_t devIndex) const;
  VkResult addSwapchain(
      uint32_t      width,
      uint32_t      height,
      VkSurfaceKHR *surface,
      uint_fast8_t  devIndex
  );

  /**
   * @brief add a GraphicsPipeline to a LogicalDevice which must have a
   * swapchain
   */
  VkResult addGraphicsPipeline(
      uint32_t                         deviceIndex,
      const std::list<ShaderDataFile> &shaderFiles
  );

  VkResult createFramebuffers(uint32_t deviceIndex = 0u)
  {
    if (devices.size() > deviceIndex) {
      DeviceBoundElements *dbe = getDBE(deviceIndex);
      GraphicsPipeline    *mainPipeline =
          dbe->pipelineManager.getMainGraphicsPipeline();

      if (!dbe->swapchains.empty() && mainPipeline) {
        return dbe->fbb.createFramebuffers(
            dbe->getDevice(),
            dbe->rpm.front().get(),
            &dbe->swapchains.front()
        );
      }
      else if (dbe->swapchains.empty()) {
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

  // TODO handle index shouldn't be using front() either indice or main
  VkResult recreateSwapchain(VkExtent2D window, uint32_t deviceIndex = 0u)
  {
    DeviceBoundElements *dbe = getDBE(deviceIndex);
    LOG_DEBUG("Device") << "SC has value " << !dbe->swapchains.empty();
    if (!dbe->swapchains.empty()) {
      GraphicsPipeline *mainPipeline =
          dbe->pipelineManager.getMainGraphicsPipeline();
      if (mainPipeline) {
        LOG_DEBUG("Pipeline") << "GP has value " << (mainPipeline != nullptr);
        mainPipeline->updateExtent(window);
      }
      dbe->swapchains.front().resizeSwapChain(
          dbe->getPhysicalDevice(),
          dbe->getDevice().get(),
          window,
          dbe->rpm.front(),
          dbe->fbb
      );

      return VK_SUCCESS;
    }
    return VK_ERROR_DEVICE_LOST;
  }
};

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag);
OSTREAM_OP(const VkQueueFamilyProperties &prop);
OSTREAM_OP(const VkExtent3D &ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
} // namespace ABox_Utils
#endif
