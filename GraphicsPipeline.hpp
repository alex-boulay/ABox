#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "SwapchainManager.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const std::array<VkDynamicState, 2> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

class GraphicsPipeline {
  VkViewport       viewport;
  VkRect2D         scissor;
  VkPipelineLayout pipelineLayout;
  const VkDevice  &device;
  // bindShaderHandlers here
  //
  GraphicsPipeline(
      const SwapchainManager &sm,
      const VkDevice         &device
  )
      : device(device)
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
    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT,
        .pNext                   = nullptr,
        .flags                   = 0u,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = 0u,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f
    };
    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable         = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1u,
        .pAttachments    = &colorBlendAttachment,
        .blendConstants  = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0u,
        .setLayoutCount         = 0u,
        .pSetLayouts            = nullptr,
        .pushConstantRangeCount = 0u,
        .pPushConstantRanges    = nullptr
    };
    VkResult res = VK_SUCCESS;
    res          = vkCreatePipelineLayout(
        device, // TODO: add Device Management to Pipeline
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout
    );
    if (res != VK_SUCCESS) {
      std::cout << "Vulkan Error value : " << res << std::endl;
      throw std::runtime_error("failed to create pipeline layout !");
    }
  }
  void CreateRenderPass() { VkAttachmentDescription colorAttachment{}; }
  ~GraphicsPipeline()
  {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
  }
};

#endif

