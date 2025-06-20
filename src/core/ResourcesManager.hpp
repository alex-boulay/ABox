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

class InstanceWrapper : public MemoryWrapper<VkInstance> {
   public:
  InstanceWrapper(
      VkInstance instance                     = VK_NULL_HANDLE,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : MemoryWrapper<VkInstance>(
            instance,
            std::function([this, pAllocator]() {
              std::cout << "Instance Wrapper destructor call" << std::endl;
              if (this->get() != VK_NULL_HANDLE) {
                vkDestroyInstance(this->get(), pAllocator);
              }
            })
        )
  {
  }
};

class SurfaceWrapper : public MemoryWrapper<VkSurfaceKHR> {
   public:
  SurfaceWrapper(
      VkInstance instance                     = VK_NULL_HANDLE,
      VkSurfaceKHR surface                    = VK_NULL_HANDLE,
      const VkAllocationCallbacks *pAllocator = nullptr

  )
      : MemoryWrapper<VkSurfaceKHR>(
            surface,
            std::function([this, instance, pAllocator]() {
              if (this->get() != VK_NULL_HANDLE && instance != VK_NULL_HANDLE) {
                std::cout << "Surface Destroyed " << (void *)this->get()
                          << std::endl;
                vkDestroySurfaceKHR(instance, this->get(), pAllocator);
              }
            })
        )
  {
    std::cout << "Surface Created : " << (void *)surface << std::endl;
  }
};

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
    ABox_Utils::DeviceBoundElements *dbe = deviceHandler->getDBE("main");

    dbe->getSyncroManagerPtr()->synchroniseDraw(
        dbe->getDevice(),
        dbe->getSyncroManagerPtr()->getFence("inFlightFence")
    );
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
    return;
  }
};

#endif // RESSOURCES_MANAGER_HPP
