#ifndef SWAPCHAIN_MANAGER_HPP
#define SWAPCHAIN_MANAGER_HPP

#include "PreProcUtils.hpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>

class SwapchainManager {

  // Main Utils
  VkSwapchainKHR           swapChain = VK_NULL_HANDLE;
  std::vector<VkImage>     swapChainImages;
  VkFormat                 swapChainImageFormat;
  VkExtent2D               swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;

  VkDevice                        device;
  std::function<VkSurfaceKHR *()> surfaceCallback;
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
      VkPhysicalDevice                phyDev,
      std::function<VkSurfaceKHR *()> surface,
      VkDevice                        logicalDevice,
      uint32_t                        rQDI,
      uint32_t                        gQDI,
      uint32_t                        width,
      uint32_t                        height
  );
  ~SwapchainManager();

  // Move constructor
  SwapchainManager(
      SwapchainManager &&other
  ) noexcept
      : swapChain(other.swapChain)
      , swapChainImages(std::move(other.swapChainImages))
      , swapChainImageFormat(other.swapChainImageFormat)
      , swapChainExtent(other.swapChainExtent)
      , swapChainImageViews(std::move(other.swapChainImageViews))
      , device(other.device)
      , surfaceCallback(std::move(other.surfaceCallback))
      , capabilities(other.capabilities)
      , formats(std::move(other.formats))
      , presentModes(std::move(other.presentModes))
      , surfaceFormat(other.surfaceFormat)
      , presentMode(other.presentMode)
      , extent(other.extent)
  {
    std::cout << "SwapchainManager Move constructor " << std::endl;
    // Null out the source object to indicate ownership transfer
    other.swapChain            = VK_NULL_HANDLE;
    other.swapChainImageFormat = VK_FORMAT_UNDEFINED;
    other.device               = VK_NULL_HANDLE;
    other.surfaceCallback      = nullptr;
  }

  // Move assignment operator
  SwapchainManager &operator=(
      SwapchainManager &&other
  ) noexcept
  {
    std::cout << "SwapchainManager Move assignment operator " << std::endl;
    if (this != &other) {
      // Free current resources if necessary (e.g., destroy swapchain)

      // Transfer resources from 'other' to 'this'
      swapChain            = other.swapChain;
      swapChainImages      = std::move(other.swapChainImages);
      swapChainImageFormat = other.swapChainImageFormat;
      swapChainExtent      = other.swapChainExtent;
      swapChainImageViews  = std::move(other.swapChainImageViews);
      device               = other.device;
      surfaceCallback      = std::move(other.surfaceCallback);
      capabilities         = other.capabilities;
      formats              = std::move(other.formats);
      presentModes         = std::move(other.presentModes);
      surfaceFormat        = other.surfaceFormat;
      presentMode          = other.presentMode;
      extent               = other.extent;

      // Null out the source object to indicate ownership transfer
      other.swapChain            = VK_NULL_HANDLE;
      other.swapChainImageFormat = VK_FORMAT_UNDEFINED;
      other.device               = VK_NULL_HANDLE;
      other.surfaceCallback      = nullptr;
    }
    return *this;
  }

  VkExtent2D getExtent() const noexcept { return extent; }

  DELETE_COPY(SwapchainManager);

  VkFormat getFormat() const noexcept { return swapChainImageFormat; }

  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  // resizeSwapChain(uint32_t width, uint32_t height);
};

#endif
