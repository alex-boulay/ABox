#ifndef SWAPCHAIN_MANAGER_HPP
#define SWAPCHAIN_MANAGER_HPP

#include "PreProcUtils.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

class SwapchainManager {

  // Main Utils
  VkDevice                 device; //= VK_NULL_HANDLE;
  VkSwapchainKHR           swapChain = VK_NULL_HANDLE;
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
  VkResult createImageViews();

   public:
  SwapchainManager(
      VkPhysicalDevice phyDev,
      VkSurfaceKHR     surface,
      VkDevice         logdev,
      uint32_t         rQDI,
      uint32_t         gQDI,
      uint32_t         width,
      uint32_t         height
  );
  ~SwapchainManager();

  // DEFAULT_COPY(SwapchainManager);
  // DEFAULT_MOVE(SwapchainManager);

  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  // resizeSwapChain(uint32_t width, uint32_t height);
};

#endif
