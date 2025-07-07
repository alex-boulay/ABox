
#ifndef QUEUE_MANAGER_HPP
#define QUEUE_MANAGER_HPP

// TODO : once done need to remove the equivalent
// parts in the deviceManager and use thooses

/**
 * maping all queue management here
 * calculation around indicies of given queues will also be done here
 */
#include "CommandsHandler.hpp"
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

// Bind Queue Roles To Shaders Management too ?
enum class QueueRole { Graphics, Present, Compute, Transfer };

class QueueManager {

  VkQueue graphicsQueue = VK_NULL_HANDLE; // move to Queue Management ??
  VkQueue presentQueue  = VK_NULL_HANDLE;

  CommandsHandler commands;

  std::unordered_map<QueueRole, uint32_t> queueFamilyIndices{
      {QueueRole::Graphics, graphicsQueueIndex},
      { QueueRole::Present,  presentQueueIndex},
      { QueueRole::Compute,  computeQueueIndex}
  };

   public:
  std::optional<uint32_t> graphicsQueueIndex;
  std::optional<uint32_t> presentQueueIndex;
  std::optional<uint32_t> computeQueueIndex;

  QueueManager(
      VkDevice dev,
  )
      : commands()
  {
  }

  bool isGraphicPresentSameQueue() const
  {
    return graphicsQueueIndex.has_value() && presentQueueIndex.has_value() &&
           graphicsQueueIndex.value() == presentQueueIndex.value();
  }
};
#endif
