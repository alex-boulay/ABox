#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
// #include "ShaderHandler.hpp"
#include <GLFW/glfw3.h>
#include <vector>
/**
 * @class ABoxApp
 * @brief Vulkan Loader application
 *
 */
class ABoxApp {
  static const uint_fast16_t WIDTH  = 800;
  static const uint_fast16_t HEIGHT = 600;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"
  };
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDebugUtilsMessengerEXT debugMessenger;
  //  ShaderHandler            shaderHandler;

  bool checkValidationLayerSupport();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  void cleanup();
  void createInstance();
  void initVulkan();
  void mainLoop();

  void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void setupDebugMessenger();

   public:
  void run();
};
