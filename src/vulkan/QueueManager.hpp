
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
#include <map>
#include <optional>
#include <vulkan/vulkan_core.h>

class QueueManager {
  std::optional<uint32_t> graphicsQueueIndex;
  std::optional<uint32_t> presentQueueIndex;
  std::optional<uint32_t> computeQueueIndex;

  VkQueue graphicsQueue = VK_NULL_HANDLE; // move to Queue Management ??
  VkQueue presentQueue  = VK_NULL_HANDLE;

  std::map<uint32_t, CommandsHandler> commandManagers;
};
#endif
