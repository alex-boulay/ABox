#include "ResourcesManager.hpp"
#include "DeviceHandler.hpp"
#include "ShaderHandler.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifdef DEBUG_VK_ABOX
#endif

std::vector<const char *> ResourcesManager::getLayerNames()
{
  std::vector<const char *> result;
  for (const char *a : InstanceLayers) {
    result.push_back(a);
  }
  return result;
}

ResourcesManager::ResourcesManager()
{
  std::cout << "before app Info " << std::endl;
  VkApplicationInfo appInfo{
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext              = nullptr,
      .pApplicationName   = "ABoxApp",
      .applicationVersion = 10000,
      .pEngineName        = "ABox",
      .engineVersion      = 10000,
      .apiVersion         = VK_API_VERSION_1_3
  };
  std::cout << "after app info " << &appInfo << std::endl;

  std::vector<const char *> extBuffer   = getExtensions();
  std::vector<const char *> layerBuffer = getLayerNames();

  for (auto a : extBuffer) {
    std::cout << "Extension : " << a << '\n';
  }

  for (auto a : layerBuffer) {
    std::cout << "Layer : " << a << '\n';
  }

  std::cout << "before debug Create info " << std::endl;
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
      debugHandler.populateDebugMessenger();

  std::cout << "after debug Create info " << std::endl;
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo,
      .flags = 0, // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo        = &appInfo,
      .enabledLayerCount       = static_cast<uint32_t>(layerBuffer.size()),
      .ppEnabledLayerNames     = layerBuffer.data(),
      .enabledExtensionCount   = static_cast<uint32_t>(extBuffer.size()),
      .ppEnabledExtensionNames = extBuffer.data()
  };

  std::cout << "after create instance info " << &instanceCreateInfo
            << std::endl;
  VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Resources Manager Error : failed to create instance! VkResult = "
       << res << std::endl;
    throw std::runtime_error(ss.str());
  }

  debugHandler = DebugHandler(instance);
  debugHandler.setupDebugMessenger();
  devices = ABox_Utils::DeviceHandler(instance);
}

std::vector<const char *> ResourcesManager::getExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    InstanceExtensions.insert(glfwExtensions[i]);
  }

  glfwExtensionCount = InstanceExtensions.size();

  return std::vector<const char *>(
      InstanceExtensions.begin(),
      InstanceExtensions.end()
  );
}

ResourcesManager::~ResourcesManager()
{
  std::cout << "Delete Call to ressourceManager" << std::endl;
  std::cout << "removing bindings" << std::endl;
  devices.removeBindings();
  std::cout << "Deleting Surface : " << surface << std::endl;
  if (instance != VK_NULL_HANDLE) {

    if (surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instance, surface, nullptr);
      surface = VK_NULL_HANDLE;
    }

    std::cout << "Deleting DebugHandler " << std::endl;
    debugHandler.~DebugHandler();
    // leak here :
    // add the vulkan debug layers to the instance
    std::cout << "Deleting instance : " << &instance << std::endl;
    vkDestroyInstance(instance, nullptr);
    instance = VK_NULL_HANDLE;
  }
}

VkResult ResourcesManager::addLogicalDevice()
{
  return devices.addLogicalDevice(surface);
}
VkResult ResourcesManager::addLogicalDevice(
    uint32_t physicalDeviceIndex
)
{
  return devices.addLogicalDevice(physicalDeviceIndex, surface);
}

VkResult ResourcesManager::createSwapchain(
    uint32_t width,
    uint32_t height,
    uint32_t devIndex
)
{
  return devices.hasDevice(devIndex)
             ? devices.addSwapchain(width, height, &this->surface, devIndex)
             : VK_ERROR_DEVICE_LOST;
}

VkResult ResourcesManager::addGraphicsPipeline(
    const std::vector<ShaderDataFile> &smcis,
    uint32_t                           deviceIndex = 0u
)
{
  return devices.hasDevice(deviceIndex)
             ? devices.addGraphicsPipeline(deviceIndex, smcis)
             : VK_ERROR_DEVICE_LOST;
}
