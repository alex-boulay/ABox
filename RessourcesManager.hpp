#ifndef RESSOURCES_MANAGER_HPP
#define RESSOURCES_MANAGER_HPP

#include "DeviceHandler.hpp"
#include "PreProcUtils.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


class RessourcesManager {
  ABox_Utils::DeviceHandler devices;
  VkInstance                instance;

   public:
  RessourcesManager();

  std::vector<const char *> getExtensions();

  ~RessourcesManager();

  ABox_Utils::DeviceHandler *getDevices() { return &devices; }
  VkInstance                 getInstance() const { return instance; }

  std::vector<const char*> getLayerNames();
  DELETE_COPY(
      RessourcesManager
  )
  DELETE_MOVE(
      RessourcesManager
  )
};

#endif // RESSOURCES_MANAGER_HPP
