#ifndef COMMANDS_HANDLER_HPP
#define COMMANDS_HANDLER_HPP

#include "GraphicsPipeline.hpp"
#include "MemoryWrapper.hpp"
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(
    VkCommandPool,
    CommandPool,
    vkDestroyCommandPool
)

// Bind Queue Roles To Shaders Management too ?
// This enum should be in the queueManager but trough dependencies calls ends
// here for the moment
enum class QueueRole { Graphics, Present, Compute, Transfer };

class CommandBoundElement {
   public:
  CommandPoolWrapper           commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  CommandBoundElement(VkDevice device, VkCommandPoolCreateInfo poolInfo);

  CommandBoundElement(
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

class CommandsHandler {
  std::list<CommandBoundElement> CBEs; // CommandBoundElements
  std::unordered_map<QueueRole, CommandBoundElement *> roleBindings;

   public:
  CommandsHandler(
      VkDevice                                       device,
      const std::unordered_map<QueueRole, uint32_t> &queueFamilyIndices,
      VkCommandPoolCreateFlags                       createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  );
};

#endif // COMMANDS_HANDLER_HPP
