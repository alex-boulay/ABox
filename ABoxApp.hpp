#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "ShaderHandler.hpp"
#include <GLFW/glfw3.h>
#include <vector>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;
};

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

  GLFWwindow              *window;
  VkInstance               instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
  VkDevice                 device;
  VkQueue                  graphicsQueue;
  VkQueue                  presentQueue;
  VkSurfaceKHR             surface;
  VkSwapchainKHR           swapChain;
  std::vector<VkImage>     swapChainImages;
  VkFormat                 swapChainImageFormat;
  VkExtent2D               swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  ShaderHandler            shaderHandler;

  bool checkValidationLayerSupport();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  void               cleanup();
  void               createInstance();
  void               createLogicalDevice();
  void               createSurface();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void               initVulkan();
  void               initWindow();
  bool               isDeviceSuitable(VkPhysicalDevice device);
  void               mainLoop();
  void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void pickPhysicalDevice();
  void setupDebugMessenger();
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR      chooseSwapSurfaceFormat(
           const std::vector<VkSurfaceFormatKHR> &availableFormats
       );
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes
  );
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void       createSwapChain();
  void       createImageViews();
  void       createGraphicsPipeline();

   public:
  void run();
};
