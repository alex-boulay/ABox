#ifndef COMMANDS_HANDLER_HPP
#define COMMANDS_HANDLER_HPP

#include "GraphicsPipeline.hpp"
#include "MemoryWrapper.hpp"
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(VkCommandPool, CommandPool, vkDestroyCommandPool)

// Bind Queue Roles To Shaders Management too ?
// This enum should be in the queueManager but trough dependencies calls ends
// here for the moment
enum class QueueRole { Graphics, Present, Compute, Transfer };

inline std::ostream &operator<<(std::ostream &os, QueueRole role)
{
  switch (role) {
    case QueueRole::Graphics: os << "Graphics"; break;
    case QueueRole::Present: os << "Present"; break;
    case QueueRole::Compute: os << "Compute"; break;
    case QueueRole::Transfer: os << "Transfer"; break;
    default: os << "Unknown"; break;
  }
  return os;
}

using QueueFamilyIndices = std::unordered_map<QueueRole, uint32_t>;

class CommandBoundElement {
   public:
  CommandPoolWrapper           commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  QueueRole                    queueRole;

  CommandBoundElement(
      VkDevice                device,
      QueueRole               qRole,
      VkCommandPoolCreateInfo poolInfo,
      uint32_t                bufferCount
  );

  CommandBoundElement(
      VkDevice                 device,
      QueueRole                qRole,
      uint32_t                 queueFamilyIndex,
      uint32_t                 bufferCount,
      VkCommandPoolCreateFlags createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  );

  // TODO : Analyse wanted behavior - Might not suit all needs
  // Behave like a start to end with a fillup -> enhance.
  VkResult createCommandBuffer(
      VkDevice             device,
      uint32_t             bufferCount,
      VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
  );

  VkResult recordCommandBuffer(
      GraphicsPipeline &gp,
      Swapchain        &sm,
      uint32_t          imageIndex,
      uint32_t          commandBufferIndex
  );

  VkCommandBuffer getCommandBuffer(uint32_t index)
  {
    return commandBuffers.at(index);
  }

  VkCommandBuffer *getCommandBufferPtr(uint32_t index)
  {
    return &commandBuffers.at(index);
  }
};

class CommandsHandler {
  std::list<CommandBoundElement> CBEs;

   public:
  CommandsHandler(
      VkDevice                                       device,
      const std::unordered_map<QueueRole, uint32_t> &queueFamilyIndices,
      VkCommandPoolCreateFlags                       createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  );

  CommandBoundElement &top() { return CBEs.back(); }
};

#endif // COMMANDS_HANDLER_HPP
