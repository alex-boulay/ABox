#include "ResourcesManager.hpp"
#include "DeviceHandler.hpp"
#include "ShaderHandler.hpp"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifdef DEBUG_VK_ABOX
#endif

std::vector<const char *> ResourcesManager::getLayerNames()
{
  std::vector<const char *> result;
  for (const char *a : InstanceLayers) {
    result.push_back(a);
  }
  return result;
}

ResourcesManager::ResourcesManager()
{
  std::cout << "before app Info " << std::endl;
  VkApplicationInfo appInfo{
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext              = nullptr,
      .pApplicationName   = "ABoxApp",
      .applicationVersion = 10000,
      .pEngineName        = "ABox",
      .engineVersion      = 10000,
      .apiVersion         = VK_API_VERSION_1_3
  };
  std::cout << "after app info " << &appInfo << std::endl;

  std::vector<const char *> extBuffer   = getExtensions();
  std::vector<const char *> layerBuffer = getLayerNames();

  for (auto a : extBuffer) {
    std::cout << "Extension : " << a << '\n';
  }

  for (auto a : layerBuffer) {
    std::cout << "Layer : " << a << '\n';
  }

  std::cout << "before debug Create info " << std::endl;
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
      debugHandler.populateDebugMessenger();

  std::cout << "after debug Create info " << std::endl;
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo,
      .flags = 0, // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo        = &appInfo,
      .enabledLayerCount       = static_cast<uint32_t>(layerBuffer.size()),
      .ppEnabledLayerNames     = layerBuffer.data(),
      .enabledExtensionCount   = static_cast<uint32_t>(extBuffer.size()),
      .ppEnabledExtensionNames = extBuffer.data()
  };

  std::cout << "after create instance info " << &instanceCreateInfo
            << std::endl;
  VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, instance.ptr());
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Resources Manager Error : failed to create instance! VkResult = "
       << res << std::endl;
    throw std::runtime_error(ss.str());
  }

  debugHandler = DebugHandler(instance.get());
  debugHandler.setupDebugMessenger();
  deviceHandler.emplace(instance.get());
  surfaces.emplace_back(instance, VK_NULL_HANDLE);
}

std::vector<const char *> ResourcesManager::getExtensions()
{
  uint32_t     glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::cout << "glfwExtensionCount : " << glfwExtensionCount << std::endl;
  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    std::cout << "\t-- Extension : " << glfwExtensions[i] << std::endl;
    InstanceExtensions.insert(glfwExtensions[i]);
  }

  return std::vector<const char *>(
      InstanceExtensions.begin(),
      InstanceExtensions.end()
  );
}

VkResult ResourcesManager::addLogicalDevice()
{
  return deviceHandler.value().addLogicalDevice(getWindowSurface());
}

VkResult ResourcesManager::addLogicalDevice(
    uint32_t    physicalDeviceIndex,
    std::string name
)
{
  return deviceHandler.value()
      .addLogicalDevice(getWindowSurface(), name, physicalDeviceIndex);
}

VkResult ResourcesManager::createSwapchain(
    uint32_t width,
    uint32_t height,
    uint32_t devIndex
)
{
  return deviceHandler.value().hasDevice(devIndex)
             ? deviceHandler.value()
                   .addSwapchain(width, height, getWindowSurfacePtr(), devIndex)
             : VK_ERROR_DEVICE_LOST;
}

VkResult ResourcesManager::addGraphicsPipeline(
    const std::list<ShaderDataFile> &smcis,
    uint32_t                         devIndex
)
{
  std::cout << " devIndex : " << devIndex << " Instance Loaded DeviceHandler ? "
            << deviceHandler.has_value() << " - device exists  "
            << deviceHandler.value().hasDevice(devIndex) << std::endl;
  std::cout << "ShaderDataFiles size : " << smcis.size() << std::endl;
  return deviceHandler.value().hasDevice(devIndex)
             ? deviceHandler.value().addGraphicsPipeline(devIndex, smcis)
             : VK_ERROR_DEVICE_LOST;
}
VkResult ResourcesManager::createFramebuffers(
    uint32_t devIndex = 0u
)
{
  if (deviceHandler.has_value()) {
    return deviceHandler.value().createFramebuffers(devIndex);
  }
  else {
    throw std::runtime_error(
        "No DeviceHandler allocated - impossible to create a framebuffer !"
    );
  }
}
void ResourcesManager::drawFrame()
{
  uint32_t                         imageIndex;
  ABox_Utils::DeviceBoundElements *dbe = deviceHandler->getDBE("main");
  std::cout << "Got main deviceHandler" << std::endl;
  dbe->getFrameSyncArray()->waitAndReset(dbe->getDevice());
  std::cout << "Wait and reset done" << std::endl;

  VkResult result = vkAcquireNextImageKHR(
      dbe->getDevice(),
      dbe->swapchain.value().getSwapchain(),
      UINT64_MAX,
      dbe->getFrameSyncArray()->getFrameSyncObject()->imageOk.get(),
      VK_NULL_HANDLE,
      &imageIndex
  );
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // dbe->swapchain.recreateSwapChain();
    std::cout << "Need to recreate Swapchain ! " << std::endl;
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  const uint32_t frameIndex = dbe->getFrameSyncArray()->getFrameIndex();
  std::cout << "ImageIndex " << imageIndex << std::endl;
  std::cout << "FrameIndex " << frameIndex << std::endl;

  dbe->recordCommandBuffer(imageIndex, frameIndex);

  std::cout << "recordCommandBuffer done : " << std::endl;

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  VkSubmitInfo submitInfo{
      .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext              = nullptr,
      .waitSemaphoreCount = 1u,
      .pWaitSemaphores =
          dbe->getFrameSyncArray()->getFrameSyncObject()->imageOk.ptr(),
      .pWaitDstStageMask  = waitStages,
      .commandBufferCount = static_cast<uint32_t>( // TODO maybe not size but
                                                   // binding to a specific one
          1u // dbe->getCommandHandler()->top().commandBuffers.size()
      ),
      .pCommandBuffers =
          dbe->getCommandHandler()->top().getCommandBufferPtr(frameIndex
          ), // TODO after bound to queue management -
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          dbe->getFrameSyncArray()->getFrameSyncObject()->renderEnd.ptr(),
  };
  result = vkQueueSubmit(
      dbe->graphicsQueue,
      1,
      &submitInfo,
      dbe->getFrameSyncArray()->getFrameSyncObject()->inFlight
  );

  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{
      .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext              = nullptr,
      .waitSemaphoreCount = 1u,
      .pWaitSemaphores =

          dbe->getFrameSyncArray()->getFrameSyncObject()->renderEnd.ptr(),
      .swapchainCount = 1u,
      .pSwapchains    = dbe->swapchain.value().swapchainPtr(),
      .pImageIndices  = &imageIndex,
      .pResults       = nullptr
  };

  vkQueuePresentKHR(dbe->presentQueue, &presentInfo);

  dbe->getFrameSyncArray()->incrementFrameIndex();
  return;
}
