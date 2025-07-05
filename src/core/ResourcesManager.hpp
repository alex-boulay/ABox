#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include "ShaderHandler.hpp"
#include <stdexcept>
#include <unordered_set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifdef DEBUG_VK_ABOX
  #include "DebugHandler.hpp"
#endif

DEFINE_VK_MEMORY_WRAPPER_SOLO(
    VkInstance,
    Instance,
    vkDestroyInstance
)

DEFINE_VK_MEMORY_WRAPPER_FULL(
    VkSurfaceKHR,
    Surface,
    vkDestroySurfaceKHR,
    VkInstance
)

class ResourcesManager {
  InstanceWrapper instance;

#ifdef DEBUG_VK_ABOX
  DebugHandler debugHandler;
#endif

  std::list<SurfaceWrapper>                surfaces;
  std::optional<ABox_Utils::DeviceHandler> deviceHandler;

  std::unordered_set<const char *> InstanceLayers = {
#ifdef VK_ABOX_VALIDATION_LAYERS
      "VK_LAYER_KHRONOS_validation",
#endif
#ifdef VK_ABOX_PROFILING
      "VK_LAYER_KHRONOS_profiles",
#endif
  };

  std::unordered_set<const char *> InstanceExtensions = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
  }; // getExtensions was added to fetch necessary functions for glfw.

   public:
  ResourcesManager();
  ~ResourcesManager()
  {
    std::cout << "ResourcesManager Destructor Call ! " << std::endl;
  }

  std::vector<const char *> getExtensions();

  inline ABox_Utils::DeviceHandler *getDeviceHandler()
  {
    return &deviceHandler.value();
  }

  //---------------------------------------------------------------
  VkInstance getInstance() { return instance.get(); }

  VkInstance *getInstancePtr() { return instance.ptr(); }

  //---------------------------------------------------------------

  VkSurfaceKHR  getWindowSurface() { return surfaces.front().get(); }
  VkSurfaceKHR *getWindowSurfacePtr() { return surfaces.front().ptr(); }

  //---------------------------------------------------------------

  VkResult addLogicalDevice();
  VkResult addLogicalDevice(uint32_t physicalDeviceIndex, std::string name);

  ABox_Utils::DeviceBoundElements *getMainDevice()
  {
    return deviceHandler->getDBE(0u);
  }

  ABox_Utils::DeviceBoundElements *getDevice(
      uint32_t device
  )
  {
    return deviceHandler->getDBE(0u);
  }
  VkResult VkResuladdLogicalDevice(uint32_t physicalDeviceIndex);

  VkResult
      createSwapchain(uint32_t width, uint32_t height, uint32_t devIndex = 0u);

  std::vector<const char *> getLayerNames();
  DELETE_COPY(
      ResourcesManager
  )

  DELETE_MOVE(
      ResourcesManager
  )

  VkResult addGraphicsPipeline(
      const std::list<ShaderDataFile> &smcis,
      uint32_t                         deviceIndex = 0u
  );

  VkResult createFramebuffers(
      uint32_t devIndex = 0u
  )
  {
    if (deviceHandler.has_value()) {
      return deviceHandler.value().createFramebuffers(devIndex);
    }
    else {
      throw std::runtime_error(
          "No DeviceHandler allocated - impossible to create a framebuffer !"
      );
    }
  }

  void drawFrame(
      uint32_t devIndex = 0u
  )
  {
    uint32_t                         imageIndex;
    ABox_Utils::DeviceBoundElements *dbe = deviceHandler->getDBE("main");
    dbe->getFrameSyncArray()->waitAndReset(dbe->getDevice());

    vkAcquireNextImageKHR(
        dbe->getDevice(),
        dbe->swapchain.value().getSwapchain(),
        UINT64_MAX,
        dbe->getFrameSyncArray()->getFrameSyncObject()->imageOk.get(),
        VK_NULL_HANDLE,
        &imageIndex
    );

    // SUBMIT -----------
    // VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSubmitInfo submitInfo{
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext              = nullptr,
        .waitSemaphoreCount = 1u,
        .pWaitSemaphores =
            dbe->getFrameSyncArray()->getFrameSyncObject()->imageOk.ptr(),
        .pWaitDstStageMask    = waitStages,
        .commandBufferCount   = 1u,
        .pCommandBuffers      =,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores =
            dbe->getFrameSyncArray()->getFrameSyncObject()->renderEnd.ptr(),
    };
    VkResult result = vkQueueSubmit(
        dbe->graphicsQueue,
        1,
        &submitInfo,
        dbe->getFrameSyncArray()->getFrameSyncObject()->inFlight
    );
    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
    /** TODO
            VkSwapchainKHR swapChains[] = {};
        VkPresentInfoKHR   presentInfo{
              .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
              .pNext              = nullptr,
              .waitSemaphoreCount = 1u,
              .pWaitSemaphores    = signalSemaphores,
              .swapchainCount     = 1u,
              .pSwapchains        = swapChains,
              .pImageIndices      = &imageIndex,
              .pResults           = nullptr
        };
        vkQueuePresentKHR(presentQueue, &presentInfo);*/

    dbe->getFrameSyncArray()->incrementFrameIndex();
    return;
  }
};

#endif // RESSOURCES_MANAGER_HPP
