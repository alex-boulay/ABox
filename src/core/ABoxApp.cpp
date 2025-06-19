#include "ABoxApp.hpp"
#include "ShaderHandler.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

void ABoxApp::run()
{
  bool app_running = true;
  while (!wm.shoudlClose()) {
    rs.drawFrame(); // TODO better management in case of no display or compute.
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
  dbe->getSyncroManagerPtr()->addFence(dbe->getDevice(), "inFlightFence");
  /**
  dbe->getSyncroManagerPtr()->addSemaphore(
      dbe->getDevice(),
      "imageAvailableSemaphore"
  );
  dbe->getSyncroManagerPtr()->addSemaphore(
      dbe->getDevice(),
      "imageAvailableSemaphore"
  );*/
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
