#include "DebugHandler.hpp"
#include <bitset>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT      *pCallbackData,
    [[maybe_unused]] void                           *pUserData
)
{

  std::bitset<sizeof(messageType)> bMessageType(messageType);

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "Type :" << bMessageType
              << "| validation layer: " << pCallbackData->pMessage << '\n';
  }

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks              *pAllocator,
    VkDebugUtilsMessengerEXT                 *pDebugMessenger
)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  return func != nullptr
             ? func(instance, pCreateInfo, pAllocator, pDebugMessenger)
             : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debugMessenger,
    const VkAllocationCallbacks *pAllocator
)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
  )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
  else {
    std::cout << "vkDestroyDebugUtilsMessengerExt wasn't destroyed : "
                 "vkGetInstanceProcAddr == nullptr"
              << std::endl;
  }
}

VkDebugUtilsMessengerCreateInfoEXT DebugHandler::populateDebugMessenger()
{
  return {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback,
      .pUserData       = nullptr
  };
}

void DebugHandler::setupDebugMessenger()
{

  VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessenger();
  // TODO : to remove here just for compilation

  VkResult result = CreateDebugUtilsMessengerEXT(
      instance,
      &createInfo,
      nullptr,
      &debugMessenger
  );
  if (result != VK_SUCCESS) {
    std::cout << "Error setting up Debug Messenger - result value : " << result
              << '\n';
    throw std::runtime_error("failed to set up debug messenger!");
  }
  else {
    std::cout << "Validation Layers Enabled !" << '\n';
  }
}

DebugHandler::DebugHandler(
    VkInstance instance
)
    : instance(instance)
{
}

DebugHandler::~DebugHandler()
{
  std::cout << "Call to DebugHandler " << (instance == VK_NULL_HANDLE)
            << std::endl;
  if (instance != VK_NULL_HANDLE) {
    std::cout << "inside destroy" << std::endl;
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      std::cout << "func  not nllptr" << std::endl;
      func(instance, debugMessenger, nullptr);
    }
    instance = VK_NULL_HANDLE;
  }
  std::cout << "debughandler done " << std::endl;
}
