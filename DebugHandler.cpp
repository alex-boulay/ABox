#include "DebugHandler.hpp"
#include <bitset>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <stdexcept>

// TODO : move validation to te ressource manage rin the instance side or make a
// debug handler with the call back
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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

// TODO: tranfert debug to Instance creation
void DebugHandler::createInstance(
    VkInstance instance
)
{
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (enableValidationLayers) {
    debugCreateInfo = populateDebugMessenger();
  }

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = enableValidationLayers
                   ? (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo
                   : nullptr,
      .flags = 0, // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo  = nullptr,
      .enabledLayerCount = enableValidationLayers *
                           static_cast<uint32_t>(validationLayers.size()),
      .ppEnabledLayerNames =
          enableValidationLayers ? validationLayers.data() : nullptr,
      .enabledExtensionCount   = 0u,
      .ppEnabledExtensionNames = nullptr
  };

  std::cout << "Instance info struct filled." << '\n';
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  std::cout << "Instance info struct result value : " << result << '\n';
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance");
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

void DebugHandler::setupDebugMessenger(
    VkInstance instance
)
{
  if (!enableValidationLayers) {
    return;
  }

  VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessenger();
  // TODO : to remove here just for compilation

  VkResult result = CreateDebugUtilsMessengerEXT(
      instance,
      &createInfo,
      nullptr,
      &debugMessenger
  );
  if (result != VK_SUCCESS) {
    std::cout << "Result value : " << result << '\n';
    throw std::runtime_error("failed to set up debug messenger!");
  }
  else {
    std::cout << "Validation Layers Enabled !" << '\n';
  }
}
