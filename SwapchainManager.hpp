#ifndef SWAPCHAIN_MANAGER_HPP
#define SWAPCHAIN_MANAGER_HPP

#include <vector>
#include <vulkan/vulkan_core.h>

class SwapchainManager {

  // Main Utils
  VkSwapchainKHR           swapChain;
  std::vector<VkImage>     swapChainImages;
  VkFormat                 swapChainImageFormat;
  VkExtent2D               swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;

  // Listings
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;

  // Picking
  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR   presentMode;
  VkExtent2D         extent;

  VkResult querySwapChainSupport(VkPhysicalDevice phyDev);
  VkResult chooseSwapSurfaceFormat();
  VkResult chooseSwapPresentMode();
  VkResult chooseSwapExtent(uint32_t width, uint32_t height);

   public:
  SwapchainManager(
      VkPhysicalDevice phyDev,
      VkSurfaceKHR     surface,
      VkDevice         device
  );

  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  // resizeSwapChain(uint32_t width, uint32_t height);
};

#endif
