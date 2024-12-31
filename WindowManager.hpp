#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

class WindowManager {
  GLFWwindow  *window;
  uint32_t     width, height;
  VkSurfaceKHR surface;
  std::string  title = "ABox";

   public:
  WindowManager(uint32_t w, uint32_t h);
  ~WindowManager();

  VkSurfaceKHR createVulkanSurface(VkInstance instance);
  void         destroyVulkanSurface(VkInstance instance);

  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

  GLFWwindow *getWindow() const { return window; }
  bool        framebufferResized = false;

  static void
      framebufferResizeCallback(GLFWwindow *window, int width_, int height_);
};
