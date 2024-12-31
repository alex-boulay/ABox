#include "ResourcesManager.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

class WindowManager {
  GLFWwindow *window;
  uint32_t    width, height;
  std::string title = "ABox";

   public:
  WindowManager(uint32_t w, uint32_t h);
  ~WindowManager();

  VkResult createSurface(ResourcesManager &rm);
  void     destroySurface();

  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

  GLFWwindow *getWindow() const { return window; }
  bool        framebufferResized = false;

  static void
      framebufferResizeCallback(GLFWwindow *window, int width_, int height_);
};
