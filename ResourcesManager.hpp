#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include <unordered_set>
#include <vulkan/vulkan.h>
#ifdef DEBUG_VK_ABOX
  #include "DebugHandler.hpp"
#endif

class ResourcesManager {
  VkInstance                instance = VK_NULL_HANDLE;
  ABox_Utils::DeviceHandler devices;

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

  ABox_Utils::DeviceHandler *getDeviceHandler() { return &devices; }
  VkInstance                 getInstance() const { return instance; }

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
      std::vector<ShaderDataFile> smcis,
      uint32_t                    deviceIndex = 0u
  );
};

#endif // RESSOURCES_MANAGER_HPP
