#include "SwapchainManager.hpp"
#include "DeviceHandler.hpp"
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

SwapchainManager::SwapchainManager(
    VkPhysicalDevice phyDev,
    VkSurfaceKHR     surface,
    VkDevice         device
)
{
  // query part can be a standalone function
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDev, surface, &capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(phyDev, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        phyDev,
        surface,
        &formatCount,
        formats.data()
    );
  }
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      phyDev,
      surface,
      &presentModeCount,
      nullptr
  );

  if (presentModeCount != 0) {
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        phyDev,
        surface,
        &presentModeCount,
        presentModes.data()
    );
  }

  if (formats.empty() || presentModes.empty()) {
    std::stringstream ss;
    ss << std::boolalpha
       << "Could not create swapchain :\n\t Formats : " << (!formats.empty())
       << "\t PresentModes : " << (!presentModes.empty()) << '\n';
    throw std::runtime_error(ss.str());
  }

  chooseSwapSurfaceFormat();
  chooseSwapPresentMode();
  chooseSwapExtent();

  // TODO : query Qfam with the device.
  // might need to bind from DeviceHandler -> DeviceMap[device]->fIndices
  ABox_Utils::QueueFamilyIndices indices;
  //= findQueueFamilies(phyDev);

  std::vector<uint32_t> queueFamilyIndices = {
      indices.graphicQueueIndex.value(),
      indices.presentQueueIndex.value()
  };

#define SCSC_Mi capabilities.minImageCount + 1
#define SCSC_Ma capabilities.maxImageCount
#define SCSC_MinC std::min(SCSC_Ma, SCSC_Mi) + !(SCSC_Ma) * (SCSC_Mi)
#define SCSC_CONCURENT                                                         \
  (indices.graphicQueueIndex.value() != indices.presentQueueIndex.value())

  VkSwapchainCreateInfoKHR createInfo{
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext            = nullptr,
      .flags            = 0,
      .surface          = surface,
      .minImageCount    = SCSC_MinC,
      .imageFormat      = surfaceFormat.format,
      .imageColorSpace  = surfaceFormat.colorSpace,
      .imageExtent      = extent,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = SCSC_CONCURENT ? VK_SHARING_MODE_CONCURRENT
                                         : VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount =
          SCSC_CONCURENT * static_cast<uint32_t>(queueFamilyIndices.size()),
      .pQueueFamilyIndices =
          SCSC_CONCURENT ? queueFamilyIndices.data() : nullptr,
      .preTransform   = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode    = presentMode,
      .clipped        = VK_TRUE,
      .oldSwapchain   = VK_NULL_HANDLE
  };

#undef SCSC_CONCURENT
#undef SCSC_Ma
#undef SCSC_Mi
#undef SCSC_MinC

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain !");
  }
  else {
    std::cout << "Swapchain created : " << (void *)&swapChain << "\n";
  }

  vkGetSwapchainImagesKHR(
      device,
      swapChain,
      &createInfo.minImageCount,
      nullptr
  );
  swapChainImages.resize(createInfo.minImageCount);
  vkGetSwapchainImagesKHR(
      device,
      swapChain,
      &createInfo.minImageCount,
      swapChainImages.data()
  );

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent      = extent;
};

VkResult SwapchainManager::chooseSwapSurfaceFormat()
{
  for (const auto &availableFormat : formats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surfaceFormat = availableFormat;
      return VK_SUCCESS;
    }
  }
  surfaceFormat = formats.at(0);
#ifdef DEBUG_VK_ABOX
  std::cout << "Could not find a suitable swapSurfaceFomat."
            << "(first one from available taken)" << std::endl;
#endif
  return VK_INCOMPLETE;
}
VkResult SwapchainManager::chooseSwapPresentMode()
{
  for (const auto &availablePresentMode : presentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = availablePresentMode;
      return VK_SUCCESS;
    }
  }
  presentMode = VK_PRESENT_MODE_FIFO_KHR;
#ifdef DEBUG_VK_ABOX
  std::cout << "Could not find a Mailbox Presentation Mode." << std::endl;
#endif
  return VK_INCOMPLETE;
}
VkResult SwapchainManager::chooseSwapExtent() { return VK_SUCCESS; }