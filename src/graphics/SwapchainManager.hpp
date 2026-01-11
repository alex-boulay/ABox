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
class FrameBufferBroker;

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

class Swapchain {
  // Main Utils
  SwapchainWrapper          swapChain;
  std::list<SwapchainImage> swapChainImages;

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
  Swapchain(
      VkPhysicalDevice phyDev,
      VkSurfaceKHR    *surface,
      VkDevice         logicalDevice,
      uint32_t         rQDI,
      uint32_t         gQDI,
      uint32_t         width,
      uint32_t         height
  );

  // Move constructor
  DELETE_COPY(Swapchain);
  DELETE_MOVE(Swapchain);

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
      VkPhysicalDevice   phyDev,
      VkDevice           device,
      VkRenderPass       rp,
      FrameBufferBroker &fbb
  );

  VkResult resizeSwapChain(
      VkPhysicalDevice   phyDev,
      VkDevice           device,
      VkExtent2D         window,
      VkRenderPass       rp,
      FrameBufferBroker &fbb
  );

  inline uint32_t getMinImageCount() const
  {
    const uint32_t SCSC_Mi = capabilities.minImageCount + 1;
    const uint32_t SCSC_Ma = capabilities.maxImageCount;
    return std::min(SCSC_Ma, SCSC_Mi) + !(SCSC_Ma) * (SCSC_Mi);
  }

  [[nodiscard]] inline uint32_t getImagesCount() const
  {
    return static_cast<uint32_t>(swapChainImages.size());
  }

  [[nodiscard]] inline std::list<SwapchainImage> &getImages()
  {
    return swapChainImages;
  }
};

class SwapchainManager {
  std::list<Swapchain> swapchains;

   public:
  Swapchain         &front() { return swapchains.front(); }
  [[nodiscard]] bool empty() const noexcept { return swapchains.empty(); }

  // Emplace methods for constructing Swapchain in-place
  template <typename... Args> Swapchain &emplace_back(Args &&...args)
  {
    return swapchains.emplace_back(std::forward<Args>(args)...);
  }

  template <typename... Args> Swapchain &emplace_front(Args &&...args)
  {
    return swapchains.emplace_front(std::forward<Args>(args)...);
  }

  // Alias for emplace_back (more intuitive naming)
  template <typename... Args> Swapchain &emplace(Args &&...args)
  {
    return emplace_back(std::forward<Args>(args)...);
  }
};

#endif
