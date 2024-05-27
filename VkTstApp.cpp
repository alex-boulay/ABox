#include "VkTstApp.hpp"
#include <GLFW/glfw3.h>
#include <cstdint> 
#include <ostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <cstring>
#include <vector>
#include <iostream>

static const uint_fast16_t WIDTH  = 800;
static const uint_fast16_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

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
  pickPhysicalDevice();
  createLogicalDevice();
}

void VkTstApp::createLogicalDevice(){
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = indices.graphicsFamily.value(),
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &queueCreateInfo,
    .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
    .ppEnabledLayerNames =enableValidationLayers ? validationLayers.data() : nullptr,
    .enabledExtensionCount = 0,
    .pEnabledFeatures = &deviceFeatures,
  };

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    throw std::runtime_error("failed to create logical device!");

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

  return indices.isComplete();
}

QueueFamilyIndices VkTstApp::findQueueFamilies(VkPhysicalDevice device){
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        indices.graphicsFamily = i;
    if (indices.isComplete()) break;
    i++;
  }
  return indices;
}

void VkTstApp::mainLoop(){
    while (!glfwWindowShouldClose(window)){
      glfwPollEvents();
      //std::cout <<"Vulkan app running" <<std::endl;
    }
}

void VkTstApp::cleanup(){
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroyInstance(instance , nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

