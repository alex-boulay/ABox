#include "ABoxApp.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <stdexcept>

void ABoxApp::run() {}

ABoxApp::ABoxApp()
{
  glfwInit();
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("Vulkan Loader or ICD not found !");
  }
  rs.getDeviceHandler()->listPhysicalDevices();
  // wm.createSurface(rs);
  // rs.addLogicalDevice();
  // rs.createSwapchain(wm.getWidth(), wm.getHeight());
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
