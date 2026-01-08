#ifndef FRAME_BUFFER_BROKER_HPP
#define FRAME_BUFFER_BROKER_HPP

#include "SwapchainManager.hpp"
#include "memory/MemoryWrapper.hpp"
#include <map>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(VkFramebuffer, Framebuffer, vkDestroyFramebuffer)

typedef struct frameBufferKey {
  const VkSwapchainKHR &swapchain;
  const VkRenderPass   &renderpass;
} frameBufferKey;

class FrameBufferBroker {

  std::map<frameBufferKey, std::vector<FramebufferWrapper>> framebuffer;

   public:
  VkResult createFramebuffers(
      VkDevice     logicalDevice,
      VkRenderPass renderPass,
      Swapchain   *swapchain
  )
  {
    uint32_t i = 0u;

    frameBufferKey key{swapchain->getSwapchain(), renderPass};
    framebuffer[key].reserve(swapchain->getImagesCount());
    VkExtent2D extent = swapchain->getExtent();
    LOG_INFO("Swapchain") << "Creating FrameBuffers - for size "
                          << swapchain->getImagesCount();
    for (auto &a : swapchain->getImages()) {
      LOG_DEBUG("Swapchain") << "FrameBuffer #" << (++i);
      VkFramebufferCreateInfo fbi{
          .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext           = nullptr,
          .flags           = 0u,
          .renderPass      = renderPass,
          .attachmentCount = 1u,
          .pAttachments    = a.imageViewWrapper.ptr(),
          .width           = extent.width,
          .height          = extent.height,
          .layers          = 1u
      };
      VkFramebuffer _swapchainFramebuffer;
      VkResult      creation_return_value = vkCreateFramebuffer(
          logicalDevice,
          &fbi,
          nullptr,
          &_swapchainFramebuffer
      );
      if (creation_return_value != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer !");
      }
      else {
        framebuffer[key].emplace_back(logicalDevice, _swapchainFramebuffer);
        LOG_DEBUG("Swapchain") << "FrameBuffer created";
      }
    }
    LOG_INFO("Swapchain") << "Done with framebuffers";
    return VK_SUCCESS;
  }

  void clear(const VkSwapchainKHR &sc, const VkRenderPass &rp)
  {
    frameBufferKey key{sc, rp};
    if (framebuffer.contains(key)) {
      framebuffer.erase(key);
    }
  }

  VkFramebuffer getFrameBuffer(
      const VkSwapchainKHR &sc,
      const VkRenderPass   &rp,
      uint32_t              index
  )
  {
    frameBufferKey key{sc, rp};

    if (framebuffer.contains(key)) {
      std::vector<FramebufferWrapper> &fbw = framebuffer.at(key);
      if (fbw.size() < index) {
        return fbw.at(index);
      }
    }
  }
};

#endif
