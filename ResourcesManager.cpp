#include "ResourcesManager.hpp"
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

static std::set<const char *> InstanceLayers = {
#ifdef VK_ABOX_VALIDATION_LAYERS
    "VK_LAYER_KHRONOS_validation",
#endif
#ifdef VK_ABOX_PROFILING
    "VK_LAYER_KHRONOS_profiles",
#endif
};

static std::set<const char *> InstanceExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
}; // getExtensions was added to fetch necessary functions for glfw.

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

  VkApplicationInfo appInfo{
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext              = nullptr,
      .pApplicationName   = "ABoxApp",
      .applicationVersion = 10000,
      .pEngineName        = "ABox",
      .engineVersion      = 10000,
      .apiVersion         = VK_API_VERSION_1_3
  };

  std::vector<const char *> extBuffer   = std::move(getExtensions());
  std::vector<const char *> layerBuffer = std::move(getLayerNames());

  for (auto a : extBuffer) {
    std::cout << "Extension : " << a << '\n';
  }

  for (auto a : layerBuffer) {
    std::cout << "Layer : " << a << '\n';
  }
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0, // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo        = &appInfo,
      .enabledLayerCount       = static_cast<uint32_t>(layerBuffer.size()),
      .ppEnabledLayerNames     = layerBuffer.data(),
      .enabledExtensionCount   = static_cast<uint32_t>(extBuffer.size()),
      .ppEnabledExtensionNames = extBuffer.data()
  };

  VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Resources Manager Error : failed to create instance! VkResult = "
       << res << std::endl;
    throw std::runtime_error(ss.str());
  }
  devices = ABox_Utils::DeviceHandler(instance);
}
std::vector<const char *> getExtensions()
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

std::vector<const char *> ResourcesManager::getExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::cout << "Instances Extensions Size : " << InstanceExtensions.size()
            << std::endl;
  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    InstanceExtensions.insert(glfwExtensions[i]);
  }

  glfwExtensionCount = InstanceExtensions.size();
  std::cout << "Instances Extensions Size : " << InstanceExtensions.size()
            << std::endl;

  return std::vector<const char *>(
      InstanceExtensions.begin(),
      InstanceExtensions.end()
  );
}

ResourcesManager::~ResourcesManager()
{
  devices.~DeviceHandler();
  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
  }
  vkDestroyInstance(instance, nullptr);
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
