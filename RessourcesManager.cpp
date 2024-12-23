#include "RessourcesManager.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

RessourcesManager::RessourcesManager() {

  VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                            .pNext = nullptr,
                            .pApplicationName = "ABoxApp",
                            .applicationVersion = 10000,
                            .pEngineName = "ABox",
                            .engineVersion = 10000,
                            .apiVersion = VK_API_VERSION_1_3};

  std::vector<const char *> extBuffer = getExtensions();

  getExtensions();
  extBuffer = getExtensions();

  for (auto a : extBuffer)
    std::cout << a << '\n';
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0u,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 0u,
      .ppEnabledLayerNames = 0u,
      .enabledExtensionCount = static_cast<uint32_t>(extBuffer.size()),
      .ppEnabledExtensionNames = extBuffer.data()};

  instanceCreateInfo.ppEnabledExtensionNames =
      glfwGetRequiredInstanceExtensions(
          &instanceCreateInfo.enabledExtensionCount);

  VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Ressource Manager Error : failed to create instance! VkResult = "
       << res << std::endl;
    throw std::runtime_error(ss.str());
  }
  devices = ABox_Utils::DeviceHandler(instance);
}
std::vector<const char *> getExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++)
    InstanceExtensions.insert(glfwExtensions[i]);

  glfwExtensionCount = InstanceExtensions.size();

  return std::vector<const char *>(InstanceExtensions.begin(),
                                   InstanceExtensions.end());
}

std::vector<const char *> RessourcesManager::getExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::cout << "Instances Extensions Size : " << InstanceExtensions.size()
            << std::endl;
  for (uint32_t i = 0; i < glfwExtensionCount; i++)
    InstanceExtensions.insert(glfwExtensions[i]);

  glfwExtensionCount = InstanceExtensions.size();
  std::cout << "Instances Extensions Size : " << InstanceExtensions.size()
            << std::endl;

  return std::vector<const char *>(InstanceExtensions.begin(),
                                   InstanceExtensions.end());
}

RessourcesManager::~RessourcesManager() {
  devices.~DeviceHandler();
  vkDestroyInstance(instance, nullptr);
}
