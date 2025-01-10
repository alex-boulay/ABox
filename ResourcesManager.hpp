#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class ResourcesManager {
  VkInstance                instance;
  ABox_Utils::DeviceHandler devices;

  // Display chain
  VkSurfaceKHR surface;

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
};

#endif // RESSOURCES_MANAGER_HPP
