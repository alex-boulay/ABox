#define GLFW_INCLUDE_VULKAN
#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

WindowManager::WindowManager(
    uint32_t w,
    uint32_t h
)
    : width(w)
    , height(h)
{
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  // Set this instance as the user pointer for the window
  glfwSetWindowUserPointer(window, this);

  // Register the framebuffer resize callback
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

WindowManager::~WindowManager()
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

VkSurfaceKHR WindowManager::createVulkanSurface(
    VkInstance instance
)
{
  VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
  return surface;
}
void WindowManager::destroyVulkanSurface(
    VkInstance instance
)
{
  vkDestroySurfaceKHR(instance, surface, nullptr);
}

void WindowManager::framebufferResizeCallback(
    GLFWwindow *window,
    int         width_,
    int         height_
)
{
  std::cout << "Resizing has been called \n"
            << "\tNew Width : " << width_ << '\n'
            << "\tNew Height : " << height_ << std::endl;
  // Recreate the Swapchain
}
