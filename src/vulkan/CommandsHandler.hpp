#ifndef COMMANDS_HANDLER_HPP
#define COMMANDS_HANDLER_HPP

#include "GraphicsPipeline.hpp"
#include "MemoryWrapper.hpp"
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
  CommandsHandler(VkDevice device, VkCommandPoolCreateInfo poolInfo);

  CommandsHandler(
      VkDevice                 device,
      uint32_t                 queueFamilyIndex,
      VkCommandPoolCreateFlags createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  );

  // TODO : Analyse wanted behavior - Might not suit all needs
  // Behave like a start to end with a fillup -> enhance.
  VkResult createCommandBuffer(
      VkDevice             device,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      uint32_t bufferCount       = 1u
  );

  VkResult recordCommandBuffer(
      GraphicsPipeline gp,
      SwapchainManager sm,
      uint32_t         imageIndex,
      uint32_t         commandBufferIndex
  );
  VkCommandBuffer getCommandBuffer(
      uint32_t index
  )
  {
    return commandBuffers.at(index);
  }
};

#endif // COMMANDS_HANDLER_HPP
