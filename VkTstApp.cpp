#include "VkTstApp.hpp"
#include <GLFW/glfw3.h>
#include <cstdint> 
#include <ostream>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <limits>
#include <algorithm>
#include <set> 
#include <vulkan/vulkan_core.h>

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

bool VkTstApp::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }
  return true;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  return func != nullptr ?
    func(instance, pCreateInfo, pAllocator, pDebugMessenger) :
    VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

std::vector<const char*> getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void VkTstApp::createSurface(){
  if(glfwCreateWindowSurface(instance, window, nullptr, &surface)!= VK_SUCCESS)
    throw std::runtime_error("Failed to create window Surface");
  else {
    std::cout << "Surface Pointer : " << &surface <<std::endl;
  }
}

void VkTstApp::createInstance(){
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Tests on Vulkan",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "No Engine",
    .engineVersion =VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_3
  };
  
  auto extensions = getRequiredExtensions();
  
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if(enableValidationLayers) populateDebugMessenger(debugCreateInfo);
  
  VkInstanceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = enableValidationLayers ? (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo : nullptr,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = enableValidationLayers * static_cast<uint32_t>(validationLayers.size()),
    .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
    .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data(),
  };
  

  std::cout << "CreateInfo struct filled."<<std::endl;
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  std::cout <<"Create info struct result value : "<<result << std::endl;
  if(result != VK_SUCCESS)
    throw std::runtime_error("failed to create instance");
}

void VkTstApp::populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
  createInfo = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT ,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback
  };
}

void VkTstApp::setupDebugMessenger(){
  if(!enableValidationLayers) return ;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessenger(createInfo);
  VkResult result = CreateDebugUtilsMessengerEXT(instance,&createInfo,nullptr, &debugMessenger);
  if(result != VK_SUCCESS){
    std::cout << "Result value : "<< result << std::endl; 
    throw std::runtime_error("failed to set up debug messenger!");
  }
  else{
        std::cout << "Validation Layers Enabled !"<< std::endl;
  }  
}
void VkTstApp::run(){
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

void VkTstApp::initWindow(){
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Test", nullptr, nullptr);
}

void VkTstApp::initVulkan(){
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
}

void VkTstApp::createLogicalDevice(){
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
  float queuePriority = 1.0f;
  for( uint32_t queueFamily : uniqueQueueFamilies){
    VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queueFamily,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
    };
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
    .pQueueCreateInfos       = queueCreateInfos.data(),
    .enabledLayerCount       = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
    .ppEnabledLayerNames     = enableValidationLayers ? validationLayers.data() : nullptr,
    .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
    .ppEnabledExtensionNames = deviceExtensions.data(),
    .pEnabledFeatures        = &deviceFeatures,
  };

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("failed to create logical device!");
  else 
    std::cout << "Logical Device Created" << std::endl;
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

void VkTstApp::pickPhysicalDevice(){
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if(!deviceCount)
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }
  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

bool VkTstApp::isDeviceSuitable(VkPhysicalDevice device){
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);
  bool swapChainAdequate = false ;
  if(extensionsSupported){
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  return indices.isComplete()&& extensionsSupported && swapChainAdequate;
}

bool VkTstApp::checkDeviceExtensionSupport(VkPhysicalDevice device){
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,nullptr);
  
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device,nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
  for(const auto& extension : availableExtensions){
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

QueueFamilyIndices VkTstApp::findQueueFamilies(VkPhysicalDevice device){
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  
  VkBool32 presentSupport;
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        indices.graphicsFamily = i;
    if (indices.isComplete()) break;
    presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
    if (presentSupport) indices.presentFamily = i;
    i++;
  }
  return indices;
}

SwapChainSupportDetails VkTstApp::querySwapChainSupport(VkPhysicalDevice device){
  SwapChainSupportDetails details;
  
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount){
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR VkTstApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

VkPresentModeKHR VkTstApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
      }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkTstApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
  if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.currentExtent;
  else{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,capabilities.minImageExtent.height,capabilities.maxImageExtent.height);
    
    return actualExtent;
  }
}

void VkTstApp::createSwapChain(){
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<uint32_t> queueFamilyIndices = {
    indices.graphicsFamily.value(),
    indices.presentFamily.value()
  };

  #define SCSC_Mi swapChainSupport.capabilities.minImageCount + 1
  #define SCSC_Ma swapChainSupport.capabilities.maxImageCount
  #define SCSC_MinC std::min(SCSC_Ma,SCSC_Mi) + !(SCSC_Ma) * (SCSC_Mi)
  #define SCSC_CONCURENT (indices.graphicsFamily != indices.presentFamily)

  VkSwapchainCreateInfoKHR createInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface,
    .minImageCount = SCSC_MinC,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = SCSC_CONCURENT ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = SCSC_CONCURENT * static_cast<uint32_t>(queueFamilyIndices.size()),
    .pQueueFamilyIndices = SCSC_CONCURENT ? queueFamilyIndices.data() : nullptr,
    .preTransform = swapChainSupport.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  #undef SCSC_CONCURENT
  #undef SCSC_Ma
  #undef SCSC_Mi
  #undef SCSC_MinC

  if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    throw std::runtime_error("failed to create swap chain !");
  else
    std::cout << "Swapchain created !!" << std::endl;

  vkGetSwapchainImagesKHR(device, swapChain, &createInfo.minImageCount,nullptr);
  swapChainImages.resize(createInfo.minImageCount);
  vkGetSwapchainImagesKHR(device, swapChain,&createInfo.minImageCount, swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void VkTstApp::createImageViews(){
  swapChainImageViews.resize(swapChainImages.size());

  constexpr VkComponentSwizzle sid = VK_COMPONENT_SWIZZLE_IDENTITY ;
  
  for (size_t i = 0; i < swapChainImages.size(); i++){
    VkImageViewCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapChainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapChainImageFormat,
      .components= { sid, sid, sid, sid},
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .layerCount = 1
      }
    };
    if(vkCreateImageView(device, &createInfo,nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create image views!");
  }
}

void VkTstApp::createGraphicsPipeline(){
  
}

void VkTstApp::mainLoop(){
  while (!glfwWindowShouldClose(window)){
    glfwPollEvents();
    //std::cout <<"Vulkan app running" <<std::endl;
  }
}

void VkTstApp::cleanup(){
  for (VkImageView imageView : swapChainImageViews)
    vkDestroyImageView(device,imageView, nullptr);

  vkDestroySwapchainKHR(device, swapChain, nullptr);
  vkDestroyDevice(device, nullptr);
  if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  std::cout << "Destroy Debug Util" << std::endl;
  vkDestroySurfaceKHR(instance,surface,nullptr);
  std::cout << "SurfaceKHR Destroyed " << std::endl;
  vkDestroyInstance(instance , nullptr);
  std::cout << "Instance Destroyed" << std::endl;
  glfwDestroyWindow(window);
  glfwTerminate();
}

