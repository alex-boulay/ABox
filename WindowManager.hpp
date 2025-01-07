#include "ResourcesManager.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

/**
 * @class WindowManager
 * @brief manage window proprieties and callbacks, handle some
 * of the calls to the ressoureManager aiming for the surface
 * used inside the window, it also manage to keep trace
 * of te width and height of the window and associated swapchain
 */
class WindowManager {
  GLFWwindow *window;
  uint32_t    width, height;
  std::string title              = "ABox";
  bool        framebufferResized = false;

   public:
  WindowManager(uint32_t w, uint32_t h);
  ~WindowManager();

  VkResult createSurface(ResourcesManager &rm) const;
  VkResult
       createSwapchain(ResourcesManager &rm, uint_fast8_t devIndex = 0) const;
  void destroySurface();

  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

  GLFWwindow *getWindow() const { return window; }

  static void
      framebufferResizeCallback(GLFWwindow *window, int width_, int height_);
};
