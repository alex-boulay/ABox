#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "SwapchainManager.hpp"
#include <array>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const std::array<VkDynamicState, 2> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

class GraphicsPipeline {
  VkViewport viewport;
  VkRect2D   scissor;
  // bindShaderHandlers here
  //
  GraphicsPipeline(
      SwapchainManager sm
  )
  {
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .vertexBindingDescriptionCount   = 0u,
        .pVertexBindingDescriptions      = nullptr, // Optional
        .vertexAttributeDescriptionCount = 0u,
        .pVertexAttributeDescriptions    = nullptr, // Optional
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0u,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    scissor = {
        .offset = {0u, 0u},
        .extent = sm.getExtent()
    };
    viewport = {
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = float(scissor.extent.width),
        .height   = float(scissor.extent.width),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0u,
        .viewportCount = 1u,
        .pViewports    = &viewport,
        .scissorCount  = 1u,
        .pScissors     = &scissor
    };
  }
};

#endif

