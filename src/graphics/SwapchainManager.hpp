#ifndef SWAPCHAIN_MANAGER_HPP
#define SWAPCHAIN_MANAGER_HPP

#include "MemoryWrapper.hpp"
#include "PreProcUtils.hpp"
#include <cstdint>
#include <list>
#include <vector>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(
    VkImageView,
    ImageView,
    vkDestroyImageView
)

DEFINE_VK_MEMORY_WRAPPER(
    VkSwapchainKHR,
    Swapchain,
    vkDestroySwapchainKHR
)

DEFINE_VK_MEMORY_WRAPPER(
    VkFramebuffer,
    Framebuffer,
    vkDestroyFramebuffer
)

class SwapchainImage {
   public:
  VkImage          image;
  ImageViewWrapper imageViewWrapper;

  SwapchainImage(
      VkImage     image,
      VkImageView imageView,
      VkDevice    device
  )
      : image(image)
      , imageViewWrapper(device, imageView)
  {
  }
};

class SwapchainManager {

  // Main Utils
  SwapchainWrapper              swapChain;
  std::list<SwapchainImage>     swapChainImages;
  std::list<FramebufferWrapper> framebuffers;

  VkFormat   swapChainImageFormat;
  VkExtent2D swapChainExtent;

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
      VkSurfaceKHR    *surface,
      VkDevice         logicalDevice,
      uint32_t         rQDI,
      uint32_t         gQDI,
      uint32_t         width,
      uint32_t         height
  );
  ~SwapchainManager() = default;

  // Move constructor
  DELETE_COPY(SwapchainManager);
  DELETE_MOVE(SwapchainManager);
  VkExtent2D getExtent() const noexcept { return extent; }
  VkFormat   getFormat() const noexcept { return swapChainImageFormat; }

  VkResult createFramebuffers(VkRenderPass renderPass, VkDevice logicalDevice);

  VkFramebuffer getFrameBuffer(
      uint32_t index
  )
  {
    std::list<FramebufferWrapper>::iterator it = framebuffers.begin();
    std::advance(it, index);
    return it->get();
  }

  uint32_t acquireNextImage(
      VkDevice    device,
      VkSemaphore imgSemaphore
  )
  {
    uint32_t imageIndex;
    vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        imgSemaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );
    return imageIndex;
  }
  VkSwapchainKHR getSwapchain() const { return swapChain.get(); }
  // TODO:
  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  // resizeSwapChain(uint32_t width, uint32_t height);
};

#endif
