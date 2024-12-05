#include "DeviceHandler.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define DELETE_COPY(X)                                                         \
  X(const X &) = delete;                                                       \
  X &operator=(const X &) = delete;

#define DELETE_MOVE(X)                                                         \
  X(X &&) = delete;                                                            \
  X &operator=(X &&) = delete;

class RessourcesManager {
  DeviceHandler devices;
  VkInstance instance;

public:
  RessourcesManager() {
    VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                              .pNext = nullptr,
                              .pApplicationName = "ABoxApp",
                              .applicationVersion = 10000,
                              .pEngineName = "ABox",
                              .engineVersion = 10000,
                              .apiVersion = VK_API_VERSION_1_3};
    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0u,
        .ppEnabledLayerNames = 0u,
        .enabledExtensionCount = 0u,
        .ppEnabledExtensionNames = 0u};
    instanceCreateInfo.ppEnabledExtensionNames =
        glfwGetRequiredInstanceExtensions(
            &instanceCreateInfo.enabledExtensionCount);
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create instance!");
    }
    devices = DeviceHandler(instance);
  }

  ~RessourcesManager() {
    devices.~DeviceHandler();
    vkDestroyInstance(instance, nullptr);
  }

  DeviceHandler *getDevices() { return &devices; }
  VkInstance getInstance() const { return instance; }

  DELETE_COPY(RessourcesManager)
  DELETE_MOVE(RessourcesManager)
};
