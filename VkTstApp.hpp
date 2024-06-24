#include <vulkan/vulkan_core.h>
#include <cstdint> 
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() &&
               presentFamily.has_value();
    }
};

/**
 * @class VkTstApp
 * @brief Vulkan Loader application
 *
 */
class VkTstApp{
	
  GLFWwindow* window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;

  bool checkValidationLayerSupport();

  void cleanup();
  void createInstance();
  void createLogicalDevice();
  void createSurface();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void initVulkan();
  void initWindow();
  bool isDeviceSuitable(VkPhysicalDevice device);
  void mainLoop();
  void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  void pickPhysicalDevice();
  void setupDebugMessenger();

  public:
    void run();
};

