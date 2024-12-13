#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include <GLFW/glfw3.h>
#include <set>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static std::set<const char *> IntanceLayers = {
#ifdef VK_ABOX_VALIDATION_LAYERS
    "VK_LAYER_KHRONOS_validation",
#endif
#ifdef VK_ABOX_PROFILING
    "VK_LAYER_KHRONOS_profiles",
#endif
};

static std::set<const char *> InstanceExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

class RessourcesManager {
  ABox_Utils::DeviceHandler devices;
  VkInstance instance;

public:
  RessourcesManager();

  std::vector<const char *> getExtensions();

  ~RessourcesManager();

  ABox_Utils::DeviceHandler *getDevices() { return &devices; }
  VkInstance getInstance() const { return instance; }

  DELETE_COPY(RessourcesManager)
  DELETE_MOVE(RessourcesManager)
};

#endif // RESSOURCES_MANAGER_HPP
