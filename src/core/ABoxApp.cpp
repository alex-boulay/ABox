#include "ABoxApp.hpp"
#include "ShaderHandler.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

void ABoxApp::run()
{
  while (!wm.shouldClose()) {
    wm.pollEvents();
    rs.drawFrame();
    if (wm.consumeFramebufferResized()) {
      rs.waitIdle();
      rs.createSwapchain(wm.getWidth(), wm.getHeight());
      rs.createFramebuffers();
    }
  }
  rs.waitIdle();
}

ABoxApp::ABoxApp()
{
  // TODO Ressource Manager should just ask for a GraphicsPipeline or Compute
  // and behave accordingly
  rs.getDeviceHandler()->listPhysicalDevices();
  std::cout << "\n --Physical Device Listed -- \n" << std::endl;
  wm.createSurface(rs);
  std::cout << "\n -- Application Display Created -- \n" << std::endl;
  rs.addLogicalDevice();
  std::cout << "\n -- Logical Device added -- \n" << std::endl;
  rs.createSwapchain(wm.getWidth(), wm.getHeight());
  std::cout << "\n -- Swapchain Created -- \n" << std::endl;
  rs.addGraphicsPipeline(shaderHandler.getShaderHandlers());
  std::cout << "\n -- Graphics Pipeline added -- \n" << std::endl;
  rs.createFramebuffers();
  std::cout << "\n -- Frame Buffers Created -- \n" << std::endl;
}

ABoxApp::~ABoxApp() { glfwTerminate(); };
