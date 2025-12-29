#include "CommandsHandler.hpp"
#include "Logger.hpp"
#include "PreProcUtils.hpp"
#include <stdexcept>
#include <unordered_map>

CommandBoundElement::CommandBoundElement(
    VkDevice                device,
    QueueRole               qRole,
    VkCommandPoolCreateInfo poolInfo,
    uint32_t                bufferCount
)
    : commandPool(device)
    , queueRole(qRole)
{
  VkResult result =
      vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.ptr());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Couldn't allocate the commandPool");
  }
  else {
    LOG_DEBUG("Vulkan") << "CommandPool allocated for queue role "
                        << static_cast<int>(qRole);
  }
  createCommandBuffer(device, bufferCount);
}

VkResult CommandBoundElement::createCommandBuffer(
    VkDevice             device,
    uint32_t             bufferCount,
    VkCommandBufferLevel level
)
{
  commandBuffers.resize(bufferCount);
  VkCommandBufferAllocateInfo alInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = commandPool.get(),
      .level              = level,
      .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
  };
  VkResult result =
      vkAllocateCommandBuffers(device, &alInfo, commandBuffers.data());
  if (result == VK_SUCCESS) {
    LOG_DEBUG("Vulkan") << "Command Buffer allocation successful - Size: "
                        << commandBuffers.size();
  }
  else {
    throw std::runtime_error("Couldn't allocate command buffer ! ");
  }
  return result;
}

CommandBoundElement::CommandBoundElement(
    VkDevice                 device,
    QueueRole                qRole,
    uint32_t                 queueFamilyIndex,
    uint32_t                 bufferCount,
    VkCommandPoolCreateFlags createFlags
)
    : CommandBoundElement(
          device,
          qRole,
          {.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
           .pNext            = nullptr,
           .flags            = createFlags,
           .queueFamilyIndex = queueFamilyIndex},
          bufferCount
      )
{
}

VkResult CommandBoundElement::recordCommandBuffer(
    GraphicsPipeline &gp,
    SwapchainManager &sm,
    uint32_t          imageIndex,
    uint32_t          commandBufferIndex
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
    ABOX_LOG_PER_FRAME << "Begin Command Buffer successful";
  }
  else {
    throw std::runtime_error("Failed to do the begin command buffer ");
  }

  VkClearValue clearColor = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};

  ABOX_LOG_PER_FRAME << "Setting up render pass";
  VkRenderPassBeginInfo renderPassInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext           = nullptr,
      .renderPass      = gp.getRenderPass(),
      .framebuffer     = sm.getFrameBuffer(imageIndex),
      .renderArea      = gp.getScissor(),
      .clearValueCount = 1u,
      .pClearValues    = &clearColor
  };
  ABOX_LOG_PER_FRAME << "Framebuffer size: " << sm.frameBufferSize();

  vkCmdBeginRenderPass(
      commandBuffers.at(commandBufferIndex),
      &renderPassInfo,
      VK_SUBPASS_CONTENTS_INLINE
  );

  ABOX_LOG_PER_FRAME << "Binding pipeline";
  vkCmdBindPipeline(
      commandBuffers.at(commandBufferIndex),
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      gp.getPipeline()
  );

  ABOX_LOG_PER_FRAME << "Setting viewport";
  vkCmdSetViewport(
      commandBuffers.at(commandBufferIndex),
      0u,
      1u,
      gp.getViewportPtr()
  );

  ABOX_LOG_PER_FRAME << "Setting scissor";
  vkCmdSetScissor(
      commandBuffers.at(commandBufferIndex),
      0u,
      1u,
      gp.getScissorPtr()
  );

  ABOX_LOG_PER_FRAME << "Drawing";
  vkCmdDraw(commandBuffers.at(commandBufferIndex), 3u, 1u, 0u, 0u);

  ABOX_LOG_PER_FRAME << "Ending render pass";
  vkCmdEndRenderPass(commandBuffers.at(commandBufferIndex));

  ABOX_LOG_PER_FRAME << "Ending command buffer";
  result = vkEndCommandBuffer(commandBuffers.at(commandBufferIndex));

  if (result == VK_SUCCESS) {
    ABOX_LOG_PER_FRAME << "vkEndCommandBuffer successful";
  }
  else {
    throw std::runtime_error("Call vkEndCommandBuffer failed");
  }

  return result;
}

//----------------------------
//---- CommandHandler --------
//----------------------------

CommandsHandler::CommandsHandler(
    VkDevice                                       device,
    const std::unordered_map<QueueRole, uint32_t> &queueFamilyIndices,
    VkCommandPoolCreateFlags                       createFlags
)
{
  LOG_DEBUG("Device") << "Start queueFamilyIndices map indexing";
  for (const auto &[role, index] : queueFamilyIndices) {
    LOG_DEBUG("Device") << "Role " << static_cast<int>(role)
                        << " - QueueFamily Index: " << index;
    if (role == QueueRole::Graphics) {
      CBEs.emplace_back(
          device,
          role,
          index,
          INFLIGHT_NUMBER_OF_ELEMENTS,
          createFlags
      );
    }
    else if (role != QueueRole::Present) { // doesn't need a Command Buffer
      CBEs.emplace_back(device, role, index, createFlags);
    }

    LOG_DEBUG("Device") << "New CBE size: " << CBEs.size();
  }
}
