#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <optional>
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};
class VkTstApp{
	
  GLFWwindow* window;
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;
  VkQueue graphicsQueue;
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

