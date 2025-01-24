#include "ABoxApp.hpp"
#include "ResourcesManager.hpp"
#include "WindowManager.hpp"
#include <GLFW/glfw3.h>
#include <cstring>
#include <vulkan/vulkan_core.h>

void ABoxApp::run() { mainLoop(); }

ABoxApp::ABoxApp()
{
  ResourcesManager rs;
  rs.getDeviceHandler()->listPhysicalDevices();
  //  WindowManager    wm(720u, 1200u);
  //  wm.createSurface(rs);
  //        rs.addLogicalDevice();
  //         rs.createSwapchain(wm.getWidth(), wm.getHeight());
}

void ABoxApp::mainLoop()
{
  int a = 0;
  int b = 1;
  while (a < 17000000) {
    b = a * b;
    a++;
  }
}

