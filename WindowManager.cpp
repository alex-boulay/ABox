#include "ResourcesManager.hpp"
#include <cstdint>
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
  // glfwTerminate();
}

void WindowManager::framebufferResizeCallback(
    [[maybe_unused]] GLFWwindow *window,
    int                          width_,
    int                          height_
)
{
  std::cout << "Resizing has been called \n"
            << "\tNew Width : " << width_ << '\n'
            << "\tNew Height : " << height_ << std::endl;
  // Recreate the Swapchain
}

VkResult WindowManager::createSurface(
    ResourcesManager &rm
) const
{
  VkResult res = glfwCreateWindowSurface(
      rm.getInstance(),
      window,
      nullptr,
      rm.getSurfacePtr()
  );
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan surface!");
  }
  return res;
}

VkResult WindowManager::createSwapchain(
    ResourcesManager &rm,
    uint_fast8_t      devIndex
) const
{
  return rm.createSwapchain(width, height, devIndex);
}
