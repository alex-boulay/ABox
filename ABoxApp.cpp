#include "ABoxApp.hpp"
#include "ResourcesManager.hpp"
#include "WindowManager.hpp"
#include <cstring>
#include <vulkan/vulkan.h>

void ABoxApp::run() { mainLoop(); }

ABoxApp::ABoxApp()
{
  ResourcesManager rs;
  //    rs.getDeviceHandler()->listPhysicalDevices();
  //  WindowManager    wm(720u, 1200u);
  //  wm.createSurface(rs);
  //        rs.addLogicalDevice();
  //         rs.createSwapchain(wm.getWidth(), wm.getHeight());
}

void ABoxApp::mainLoop() {}

