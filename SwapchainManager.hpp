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

/**
class ImageViewWrapper : public MemoryWrapper<VkImageView> {
   public:
  ImageViewWrapper(
      VkImageView                  imageView,
      VkDevice                     dev,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : MemoryWrapper<VkImageView>(
            imageView,
            std::function([this, dev, pAllocator]() {
              if (dev != VK_NULL_HANDLE && this->get() != VK_NULL_HANDLE) {
                vkDestroyImageView(dev, this->get(), pAllocator);
              }
            })
        )
  {
  }
};*/
DEFINE_VK_MEMORY_WRAPPER(
    VkSwapchainKHR,
    Swapchain,
    vkDestroySwapchainKHR
)

/**
class SwapchainWrapper : public MemoryWrapper<VkSwapchainKHR> {
   public:
  SwapchainWrapper(
      VkDevice       dev,
      VkSwapchainKHR sc                       = VK_NULL_HANDLE,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : MemoryWrapper<VkSwapchainKHR>(
            sc,
            std::function([this, dev, pAllocator]() -> void {
              vkDestroySwapchainKHR(dev, this->get(), pAllocator);
            })
        )
  {
  }
};*/

DEFINE_VK_MEMORY_WRAPPER(
    VkFramebuffer,
    Framebuffer,
    vkDestroyFramebuffer
)

struct SwapchainImage {
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
  SwapchainWrapper          swapChain;
  std::list<SwapchainImage> swapChainImages;
  VkFormat                  swapChainImageFormat;
  VkExtent2D                swapChainExtent;

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

  // TODO:
  // Latter implementation for windowcallback
  // windowManager::resize(SwapchainManager sm): VkResult
  // resizeSwapChain(uint32_t width, uint32_t height);
};

#endif
