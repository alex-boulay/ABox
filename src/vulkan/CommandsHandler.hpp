#ifndef COMMANDS_HANDLER_HPP
#define COMMANDS_HANDLER_HPP

#include "GraphicsPipeline.hpp"
#include "MemoryWrapper.hpp"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(
    VkCommandPool,
    CommandPool,
    vkDestroyCommandPool
)

class CommandsHandler {

  CommandPoolWrapper           commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

   public:
  CommandsHandler(
      VkDevice                device,
      VkCommandPoolCreateInfo poolInfo
  )
      : commandPool(device)
  {
    VkResult result =
        vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.ptr());
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Couldn't allocate the commandPool");
    }
    else {
      std::cout << " CommandPool allocated ! " << std::endl;
    }
  }

  CommandsHandler(
      VkDevice                 device,
      uint32_t                 queueFamilyIndex,
      VkCommandPoolCreateFlags createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  )
      : CommandsHandler(
            device,
            {.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
             .pNext            = nullptr,
             .flags            = createFlags,
             .queueFamilyIndex = queueFamilyIndex}
        )
  {
  }
  // TODO : Analyse wanted behavior - Might not suit all needs
  // Behave like a start to end with a fillup -> enhance.
  VkResult createCommandBuffer(
      VkDevice             device,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      uint32_t bufferCount       = 1u
  )
  {
    VkCommandBufferAllocateInfo alInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = commandPool.get(),
        .level              = level,
        .commandBufferCount = bufferCount
    };
    VkCommandBuffer buffer;
    VkResult        result = vkAllocateCommandBuffers(device, &alInfo, &buffer);
    if (result == VK_SUCCESS) {
      commandBuffers.push_back(buffer);
      std::cout << "Command Buffer allocation Sucessfull" << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't allocate command buffer ! ");
    }
    return result;
  }
  VkResult recordCommandBuffer(
      GraphicsPipeline gp,
      SwapchainManager sm,
      uint32_t         imageIndex,
      uint32_t         commandBufferIndex
  )
  {
    VkCommandBufferBeginInfo beginInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = 0u,
        .pInheritanceInfo = nullptr
    };

    VkResult result =
        vkBeginCommandBuffer(commandBuffers.at(commandBufferIndex), &beginInfo);
    if (result == VK_SUCCESS) {
      std::cout << "Begin Command Buffer Sucessfull" << std::endl;
    }
    else {
      throw std::runtime_error("Failed to do the begin command buffer ");
    }

    VkClearValue clearColor = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext           = nullptr,
        .renderPass      = gp.getRenderPass(),
        .framebuffer     = sm.getFrameBuffer(imageIndex),
        .renderArea      = gp.getScissor(),
        .clearValueCount = 1u,
        .pClearValues    = &clearColor
    };
    vkCmdBeginRenderPass(
        commandBuffers.at(commandBufferIndex),
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );
    vkCmdBindPipeline(
        commandBuffers.at(commandBufferIndex),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        gp.getPipeline()
    );

    vkCmdSetViewport(
        commandBuffers.at(commandBufferIndex),
        0u,
        1u,
        gp.getViewportPtr()
    );

    vkCmdSetScissor(
        commandBuffers.at(commandBufferIndex),
        0u,
        1u,
        gp.getScissorPtr()
    );

    vkCmdEndRenderPass(commandBuffers.at(commandBufferIndex));

    result = vkEndCommandBuffer(commandBuffers.at(commandBufferIndex));
    if (result == VK_SUCCESS) {
      std::cout << "Call  vkEndCommandBuffer went through ! " << std::endl;
    }
    else {
      throw std::runtime_error("Call vkEndCommandBuffer failed");
    }

    return result;
  }
};

#endif // COMMANDS_HANDLER_HPP
