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

	void createInstance();
	void initWindow();
	void initVulkan();
  void setupDebugMessenger();
  void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
  void mainLoop();
	void cleanup();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

  bool checkValidationLayerSupport();

	public:
	void run();
};

