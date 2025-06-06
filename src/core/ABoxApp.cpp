#include "ABoxApp.hpp"
#include "ShaderHandler.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

void ABoxApp::run()
{
  bool app_running = true;
  while (app_running) {
    app_running = false;
  }
}

ABoxApp::ABoxApp()
{
  // TODO Ressource Manager should just ask for a GraphicsPipeline or Compute
  // and behave accordingly
  rs.getDeviceHandler()->listPhysicalDevices();
  wm.createSurface(rs);
  rs.addLogicalDevice();
  rs.createSwapchain(wm.getWidth(), wm.getHeight());
  rs.addGraphicsPipeline(shaderHandler.getShaderHandlers());
  rs.createFramebuffers();
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
