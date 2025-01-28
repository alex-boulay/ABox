#include "ABoxApp.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

void ABoxApp::run() {}

ABoxApp::ABoxApp()
{
  rs.getDeviceHandler()->listPhysicalDevices();
  wm.createSurface(rs);
  rs.addLogicalDevice();
  rs.createSwapchain(wm.getWidth(), wm.getHeight());
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
