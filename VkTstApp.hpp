#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
class VkTstApp{
	
	GLFWwindow* window;
	VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
	
	void createInstance();
	void initWindow();
	void initVulkan();
  void setupDebugMessenger();
  void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void mainLoop();
	void cleanup();
	
  bool checkValidationLayerSupport();

	public:
	void run();
};

