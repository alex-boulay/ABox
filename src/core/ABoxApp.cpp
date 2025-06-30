#include "ABoxApp.hpp"
#include "ShaderHandler.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

void ABoxApp::run()
{
  while (!wm.shouldClose()) {
    wm.pollEvents();
    rs.drawFrame(); // TODO better management in case of no display or compute.
    // std::cout << "FRAMING " << std::endl;
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
  ABox_Utils::DeviceBoundElements *dbe = rs.getMainDevice();
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
