#ifndef RENDER_PASS_MANAGER_HPP
#define RENDER_PASS_MANAGER_HPP

#include <deque>
#include <graphics/SwapchainManager.hpp>
#include <map>
#include <memory/MemoryWrapper.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(VkRenderPass, RenderPass, vkDestroyRenderPass)

class RenderPassManager {

  std::deque<RenderPassWrapper> rPasses;
  // std::map<std::string, RenderPassWrapper &> rpMap;

   public:
  RenderPassManager() {}

  RenderPassWrapper &front() { return rPasses.front(); }

  [[nodiscard]] bool empty() const noexcept { return rPasses.empty(); }
  VkResult           CreateRenderPass(
                VkDevice device,
                VkFormat format
            ) // format should be the swapchain one
  {
    VkAttachmentDescription colorAttachment{
        .flags          = 0u,
        .format         = format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass{
        .flags                   = 0u,
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0u,
        .pInputAttachments       = nullptr,
        .colorAttachmentCount    = 1u,
        .pColorAttachments       = &colorAttachmentRef,
        .pResolveAttachments     = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0u,
        .pPreserveAttachments    = nullptr
    };
    VkRenderPassCreateInfo renderPassInfo{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = 0u,
        .attachmentCount = 1u,
        .pAttachments    = &colorAttachment,
        .subpassCount    = 1u,
        .pSubpasses      = &subpass,
        .dependencyCount = 0u,
        .pDependencies   = nullptr
    };
    // Pileup with names and locations
    VkRenderPass renderpass;
    VkResult     res =
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass);

    if (res != VK_SUCCESS) {
      std::stringstream ss;
      ss << "Failed to create render pass !\n\tError value : " << res
         << std::endl;
      throw std::runtime_error(ss.str().c_str());
    }

    return res;
  }
};

#endif
