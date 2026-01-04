#ifndef SWAPCHAIN_MANAGER_HPP
#define SWAPCHAIN_MANAGER_HPP

#include "MemoryWrapper.hpp"
#include "PreProcUtils.hpp"
#include <array>
#include <cstdint>
#include <list>
#include <vector>
#include <vulkan/vulkan_core.h>

class GraphicsPipeline;

DEFINE_VK_MEMORY_WRAPPER(VkImageView, ImageView, vkDestroyImageView)

DEFINE_VK_MEMORY_WRAPPER(VkSwapchainKHR, Swapchain, vkDestroySwapchainKHR)

class DisplayQueueIndices {
  std::array<uint32_t, 2u> indices{0, 0};

   public:
  DisplayQueueIndices(uint32_t rQDI, uint32_t gQDI)
      : indices{rQDI, gQDI}
  {
  }

  void setIndices(uint32_t rQDI_, uint32_t gQDI_)
  {
    indices[0] = rQDI_;
    indices[1] = gQDI_;
  }

  uint32_t getRenderQueueDeviceIndice() const { return indices[0]; };

  uint32_t getGraphicsQueueDeviceIndice() const { return indices[1]; };

  uint32_t *data() { return indices.data(); }

  uint32_t size() { return static_cast<uint32_t>(indices.size()); }
};

class SwapchainImage {
   public:
  VkImage          image;
  ImageViewWrapper imageViewWrapper;

  SwapchainImage(VkImage image, VkImageView imageView, VkDevice device)
      : image(image)
      , imageViewWrapper(device, imageView)
  {
  }
};

class SwapchainManager {
  // Main Utils
  SwapchainWrapper          swapChain;
  std::list<SwapchainImage> swapChainImages;
  // std::list<FramebufferWrapper> framebuffers;

  VkExtent2D    extent;
  VkSurfaceKHR *surface;

  // Listings
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   presentModes;
  DisplayQueueIndices             queueFamilyIndices;

  // Picking
  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR   presentMode;

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

  //~SwapchainManager() = default;

  // Move constructor
  DELETE_COPY(SwapchainManager);
  DELETE_MOVE(SwapchainManager);

  VkExtent2D getExtent() const noexcept { return extent; }

  VkFormat getFormat() const noexcept { return surfaceFormat.format; }

  uint32_t getRenderQueueDeviceIndice() const
  {
    return queueFamilyIndices.getRenderQueueDeviceIndice();
  };

  uint32_t getGraphicsQueueDeviceIndice() const
  {
    return queueFamilyIndices.getGraphicsQueueDeviceIndice();
  };

  VkResult createSwapchain(VkPhysicalDevice phyDev, VkDevice logicalDevice);

  VkResult createImageViews(VkDevice device);

  uint32_t acquireNextImage(VkDevice device, VkSemaphore imgSemaphore)
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

  VkSwapchainKHR *swapchainPtr() { return swapChain.ptr(); }

  VkResult resizeSwapChain(
      VkPhysicalDevice phyDev,
      VkDevice         device,
      VkRenderPass     rp
  );

  VkResult resizeSwapChain(
      VkPhysicalDevice phyDev,
      VkDevice         device,
      VkExtent2D       window,
      VkRenderPass     rp
  );

  // TODO:
  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  VkResult resizeSwapChain(
      VkPhysicalDevice phyDev,
      VkDevice         device,
      VkExtent2D       window
  );

  inline uint32_t frameBufferSize() { return framebuffers.size(); }

  inline uint32_t getMinImageCount() const
  {
    const uint32_t SCSC_Mi = capabilities.minImageCount + 1;
    const uint32_t SCSC_Ma = capabilities.maxImageCount;
    return std::min(SCSC_Ma, SCSC_Mi) + !(SCSC_Ma) * (SCSC_Mi);
  }
};

#endif
