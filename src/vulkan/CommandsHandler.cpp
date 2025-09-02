#include "CommandsHandler.hpp"
#include "PreProcUtils.hpp"
#include <stdexcept>
#include <unordered_map>

CommandBoundElement::CommandBoundElement(VkDevice                device,
                                         QueueRole               qRole,
                                         VkCommandPoolCreateInfo poolInfo,
                                         uint32_t                bufferCount)
    : commandPool(device)
    , queueRole(qRole)
{
    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, commandPool.ptr());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Couldn't allocate the commandPool");
    }
    else
    {
        std::cout << " CommandPool allocated ! " << std::endl;
    }
    createCommandBuffer(device, bufferCount);
}

VkResult CommandBoundElement::createCommandBuffer(VkDevice device, uint32_t bufferCount, VkCommandBufferLevel level)
{
    commandBuffers.resize(bufferCount);
    VkCommandBufferAllocateInfo alInfo{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                       .pNext              = nullptr,
                                       .commandPool        = commandPool.get(),
                                       .level              = level,
                                       .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};
    VkResult                    result = vkAllocateCommandBuffers(device, &alInfo, commandBuffers.data());
    if (result == VK_SUCCESS)
    {
        std::cout << "Command Buffer allocation Sucessfull - Size : " << commandBuffers.size() << std::endl;
    }
    else
    {
        throw std::runtime_error("Couldn't allocate command buffer ! ");
    }
    return result;
}

CommandBoundElement::CommandBoundElement(VkDevice                 device,
                                         QueueRole                qRole,
                                         uint32_t                 queueFamilyIndex,
                                         uint32_t                 bufferCount,
                                         VkCommandPoolCreateFlags createFlags)
    : CommandBoundElement(device,
                          qRole,
                          {.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                           .pNext            = nullptr,
                           .flags            = createFlags,
                           .queueFamilyIndex = queueFamilyIndex},
                          bufferCount)
{
}

VkResult CommandBoundElement::recordCommandBuffer(GraphicsPipeline &gp,
                                                  SwapchainManager &sm,
                                                  uint32_t          imageIndex,
                                                  uint32_t          commandBufferIndex)
{
    VkCommandBufferBeginInfo beginInfo{.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                       .pNext            = nullptr,
                                       .flags            = 0u,
                                       .pInheritanceInfo = nullptr};

    VkResult                 result = vkBeginCommandBuffer(commandBuffers.at(commandBufferIndex), &beginInfo);

    if (result == VK_SUCCESS)
    {
        ABOX_PER_FRAME_DEBUG_LOG("Begin Command Buffer Sucessfull");
    }
    else
    {
        throw std::runtime_error("Failed to do the begin command buffer ");
    }

    VkClearValue clearColor = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};

    ABOX_PER_FRAME_DEBUG_LOG("1");
    VkRenderPassBeginInfo renderPassInfo{.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                         .pNext           = nullptr,
                                         .renderPass      = gp.getRenderPass(),
                                         .framebuffer     = sm.getFrameBuffer(imageIndex),
                                         .renderArea      = gp.getScissor(),
                                         .clearValueCount = 1u,
                                         .pClearValues    = &clearColor};
    ABOX_PER_FRAME_DEBUG_LOG("2 " << sm.frameBufferSize());

    vkCmdBeginRenderPass(commandBuffers.at(commandBufferIndex), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ABOX_PER_FRAME_DEBUG_LOG("3");
    vkCmdBindPipeline(commandBuffers.at(commandBufferIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, gp.getPipeline());

    ABOX_PER_FRAME_DEBUG_LOG("4");
    vkCmdSetViewport(commandBuffers.at(commandBufferIndex), 0u, 1u, gp.getViewportPtr());

    ABOX_PER_FRAME_DEBUG_LOG("5");
    vkCmdSetScissor(commandBuffers.at(commandBufferIndex), 0u, 1u, gp.getScissorPtr());

    ABOX_PER_FRAME_DEBUG_LOG("6");
    vkCmdDraw(commandBuffers.at(commandBufferIndex), 3u, 1u, 0u, 0u);

    ABOX_PER_FRAME_DEBUG_LOG("7");
    vkCmdEndRenderPass(commandBuffers.at(commandBufferIndex));

    ABOX_PER_FRAME_DEBUG_LOG("8");
    result = vkEndCommandBuffer(commandBuffers.at(commandBufferIndex));

    if (result == VK_SUCCESS)
    {
        ABOX_PER_FRAME_DEBUG_LOG("Call  vkEndCommandBuffer went through ! ");
    }
    else
    {
        throw std::runtime_error("Call vkEndCommandBuffer failed");
    }

    return result;
}

//----------------------------
//---- CommandHandler --------
//----------------------------

CommandsHandler::CommandsHandler(VkDevice                                       device,
                                 const std::unordered_map<QueueRole, uint32_t> &queueFamilyIndices,
                                 VkCommandPoolCreateFlags                       createFlags)
{
    std::cout << "Start queueFamilyIndices map indexing " << std::endl;
    for (const auto &[role, index] : queueFamilyIndices)
    {
        std::cout << "Role " << role << " - QueueFamily Index : " << index << std::endl;
        if (role == QueueRole::Graphics)
        {
            CBEs.emplace_back(device, role, index, INFLIGHT_NUMBER_OF_ELEMENTS, createFlags);
        }
        else if (role != QueueRole::Present)
        {    // doesn't need a Command Buffer
            CBEs.emplace_back(device, role, index, createFlags);
        }

        std::cout << "New CBE size : " << CBEs.size() << std::endl;
    }
}
