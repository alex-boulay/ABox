#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class ResourcesManager {
  ABox_Utils::DeviceHandler devices;
  VkInstance                instance;
  VkSurfaceKHR              surface;

   public:
  ResourcesManager();

  std::vector<const char *> getExtensions();

  ~ResourcesManager();

  ABox_Utils::DeviceHandler *getDevices() { return &devices; }
  VkInstance                 getInstance() const { return instance; }

  VkSurfaceKHR *getSurfacePtr() { return &surface; }
  VkResult      addLogicalDevice();
  VkResult      addLogicalDevice(uint32_t physicalDeviceIndex);

  std::vector<const char *> getLayerNames();
  DELETE_COPY(
      ResourcesManager
  )
  DELETE_MOVE(
      ResourcesManager
  )
};

#endif // RESSOURCES_MANAGER_HPP
