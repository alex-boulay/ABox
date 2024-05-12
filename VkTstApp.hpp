#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
class VkTstApp{
	
	GLFWwindow* window;
	VkInstance instance;
	
	void createInstance();
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();
	
  bool checkValidationLayerSupport();

	public:
	void run();
};

