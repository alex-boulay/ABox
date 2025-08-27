#include "ResourcesManager.hpp"
#include <cstdint>
#include <sstream>
#define GLFW_INCLUDE_VULKAN
#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

WindowManager::WindowManager(
    VkExtent2D ext
)
    : extent(ext)
{
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("Vulkan Loader or ICD not found !");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window =
      glfwCreateWindow(ext.width, ext.height, title.c_str(), nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  // Set this instance as the user pointer for the window
  glfwSetWindowUserPointer(window, this);

  // Register the framebuffer resize callback
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

WindowManager::~WindowManager()
{
  std::cout << "destroying window manager" << std::endl;
  glfwDestroyWindow(window);
  glfwTerminate();
}

void WindowManager::framebufferResizeCallback(
    [[maybe_unused]] GLFWwindow *window,
    int                          width,
    int                          height
)
{
  WindowManager *wm = (WindowManager *)(window);
  wm->extent        = VkExtent2D{
             .width  = static_cast<uint32_t>(width),
             .height = static_cast<uint32_t>(height)
  };
  wm->framebufferResized = true;
  // window.getDeviceHandler()->recreateSwapchain(extent);
#ifdef DEBUG_VK_ABOX
  std::cout << "Resizing has been called \n"
            << "\tNew Width : " << wm->getWidth() << '\n'
            << "\tNew Height : " << wm->getHeight() << std::endl;
#endif
}

VkResult WindowManager::createSurface(
    ResourcesManager &rm
) const
{
  VkResult res = glfwCreateWindowSurface(
      rm.getInstance(),
      window,
      nullptr,
      rm.getWindowSurfacePtr()
  );
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    std::cout << "getInstance value " << rm.getInstance() << std::endl;
    ss << "Failed to create Vulkan surface! res value : " << (int32_t)res
       << std::endl;
    throw std::runtime_error(ss.str().c_str());
  }
  return res;
}

VkResult WindowManager::createSwapchain(
    ResourcesManager &rm,
    uint_fast8_t      devIndex
) const
{
  return rm.createSwapchain(extent.width, extent.height, devIndex);
}
