#include "SwapchainManager.hpp"
#include "DeviceHandler.hpp"
#include <algorithm>
#include <cstdint>
#include <ios>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

/**
SwapchainManager::SwapchainManager()
    : swapChain(VK_NULL_HANDLE)
    , device(VK_NULL_HANDLE) {};
*/
SwapchainManager::SwapchainManager(
    VkPhysicalDevice                phyDev,
    std::function<VkSurfaceKHR *()> surfaceC,
    VkDevice                        logicalDevice,
    uint32_t                        rQDI,
    uint32_t                        gQDI,
    uint32_t                        width,
    uint32_t                        height
)
    : device(logicalDevice)
{
  VkSurfaceKHR surface = *surfaceC();
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
  chooseSwapExtent(width, height);

  std::vector<uint32_t> queueFamilyIndices = {rQDI, gQDI};

#define SCSC_Mi capabilities.minImageCount + 1
#define SCSC_Ma capabilities.maxImageCount
#define SCSC_MinC std::min(SCSC_Ma, SCSC_Mi) + !(SCSC_Ma) * (SCSC_Mi)
#define SCSC_CONCURENT (rQDI != gQDI)

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
    std::cout << "Swapchain created : " << (void *)swapChain << "\n";
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

  createImageViews();
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
  std::cout << "size of present mode " << presentModes.size() << std::endl;
  for (const auto &availablePresentMode : presentModes) {
    std::cout << availablePresentMode << std::endl;
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

VkResult SwapchainManager::chooseSwapExtent(
    uint32_t width,
    uint32_t height
)
{

  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    extent = capabilities.currentExtent;
  }
  else {
    extent = {
        std::clamp(
            width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        ),
        extent.height = std::clamp(
            height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        )
    };
  }
  return VK_SUCCESS;
}

VkResult SwapchainManager::createImageViews()
{
  VkResult return_value = VK_INCOMPLETE;

  swapChainImageViews.resize(swapChainImages.size());
  constexpr VkComponentSwizzle sid = VK_COMPONENT_SWIZZLE_IDENTITY;

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo = {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext      = nullptr,
        .flags      = 0,
        .image      = swapChainImages[i],
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = swapChainImageFormat,
        .components = {sid, sid, sid, sid},
        .subresourceRange =
            {.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                       .baseMipLevel   = 0,
                       .levelCount     = 1,
                       .baseArrayLayer = 0,
                       .layerCount     = 1}
    };
    return_value = vkCreateImageView(
        device,
        &createInfo,
        nullptr,
        &swapChainImageViews[i]
    );
    if (return_value != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
    else {
      std::cout << "Image Views created \n" << std::endl;
    }
  }
  return return_value;
}

SwapchainManager::~SwapchainManager()
{
  std::cout << "Swapchain Destruction Start" << std::endl;
  if (device != VK_NULL_HANDLE) {
    std::cout << "swapchain handle : " << swapChain
              << " - device handle : " << device << std::endl;
    if (swapChain != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {

      std::cout << "Image views size : " << swapChainImageViews.size() << '\n';
      for (auto imageView : swapChainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
          vkDestroyImageView(device, imageView, nullptr);
          std::cout << "ImageView Destroyed" << std::endl;
        }
        else {
          std::cout << "Image View Not destroyed" << std::endl;
        }
      }
      std::cout << "Destroying SwapchainKHR " << std::endl;
      std::cout << swapChain << " : " << &swapChain << std::endl;
      vkDestroySwapchainKHR(device, swapChain, nullptr);
      swapChain = VK_NULL_HANDLE;
    }
  }
}

