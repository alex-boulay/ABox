#ifndef FRAME_BUFFER_BROKER_HPP
#define FRAME_BUFFER_BROKER_HPP

#include <map>
#include <memory/MemoryWrapper.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(VkFramebuffer, Framebuffer, vkDestroyFramebuffer)

typedef struct frameBufferKey {
  VkSwapchainKHR &swapchain;
  VkRenderPass   &renderpass;
} frameBufferKey;

class FrameBufferBroker {

  std::map<frameBufferKey, std::vector<FramebufferWrapper>> framebuffer;

  VkResult createFramebuffers(VkRenderPass renderPass, VkDevice logicalDevice)
  {
    uint32_t i = 0u;
    LOG_INFO("Swapchain") << "Creating FrameBuffers - for size "
                          << swapChainImages.size();
    for (auto &a : swapChainImages) {
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
        framebuffers.emplace_back(logicalDevice, _swapchainFramebuffer);
        LOG_DEBUG("Swapchain") << "FrameBuffer created";
      }
    }
    LOG_INFO("Swapchain") << "Done with framebuffers";
    return VK_SUCCESS;
  }

  VkFramebuffer getFrameBuffer(uint32_t index)
  {
    std::list<FramebufferWrapper>::iterator it = framebuffers.begin();
    std::advance(it, index);
    return it->get();
  }
};

#endif
