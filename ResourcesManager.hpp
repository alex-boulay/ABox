#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include "ShaderHandler.hpp"
#include <unordered_set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#ifdef DEBUG_VK_ABOX
  #include "DebugHandler.hpp"
#endif

class InstanceWrapper : public MemoryWrapper<VkInstance> {
   public:
  InstanceWrapper(
      VkInstance                   instance,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : MemoryWrapper<VkInstance>(instance, std::function([&]() {
                                    vkDestroyInstance(instance, pAllocator);
                                  }))
  {
  }
};
class SurfaceWrapper : public MemoryWrapper<VkSurfaceKHR> {
   public:
  SurfaceWrapper(
      VkSurfaceKHR                 surface,
      VkInstance                   instance,
      const VkAllocationCallbacks *pAllocator = nullptr

  )
      : MemoryWrapper<VkSurfaceKHR>(
            surface,
            std::function([&]() {
              vkDestroySurfaceKHR(instance, surface, pAllocator);
            })
        )
  {
  }
};

class ResourcesManager {
  VkInstance                               instance = VK_NULL_HANDLE;
  std::optional<ABox_Utils::DeviceHandler> deviceHandler;

  // Display chain
  VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef DEBUG_VK_ABOX
  DebugHandler debugHandler;
#endif

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

  std::vector<const char *> getExtensions();

  ~ResourcesManager();

  inline ABox_Utils::DeviceHandler *getDeviceHandler()
  {
    return &deviceHandler.value();
  }

  VkInstance getInstance() const { return instance; }

  VkSurfaceKHR *getSurfacePtr() { return &surface; }
  VkResult      addLogicalDevice();
  VkResult      addLogicalDevice(uint32_t physicalDeviceIndex);

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
};

#endif // RESSOURCES_MANAGER_HPP
