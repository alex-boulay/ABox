#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
//
#include "GraphicsPipeline.hpp"

GraphicsPipeline::GraphicsPipeline(
    const SwapchainManager                      &sm,
    VkDevice                                     device,
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
)
    : graphicsPipeline(device)
    , renderPass(device)
    , pipelineLayout(device)
{
  std::cout << "Device value :" << (void *)device << std::endl;
  VkResult res = CreateRenderPass(sm, device);

  VkPipelineDynamicStateCreateInfo dynamicState{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext             = nullptr,
      .flags             = 0u,
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
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0u,
      .viewportCount = 1u,
      .pViewports    = &viewport,
      .scissorCount  = 1u,
      .pScissors     = &scissor
  };
  VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0u,
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
      .blendEnable         = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp        = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp        = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,

  };

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0u,
      .logicOpEnable = VK_FALSE,
      .logicOp       = VK_LOGIC_OP_COPY,
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
  res = vkCreatePipelineLayout(
      device,
      &pipelineLayoutInfo,
      nullptr,
      pipelineLayout.ptr()
  );
  if (res != VK_SUCCESS) {
    std::cout << "Vulkan Error value : " << res << std::endl;
    throw std::runtime_error("failed to create pipeline layout !");
  }
  std::cout << "Shader Stages loading into Pipeline Info " << std::endl;
  for (const VkPipelineShaderStageCreateInfo &a : shaderStages) {
    std::cout << "\tStage : " << a.stage << " - entry point : \""
              << std::string(a.pName) << "\"" << std::endl;
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext      = nullptr,
      .flags      = 0u,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages    = shaderStages.data(), // Extract from Shader Handler class
      .pVertexInputState   = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pTessellationState  = nullptr,
      .pViewportState      = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState   = &multisampling,
      .pDepthStencilState  = nullptr,
      .pColorBlendState    = &colorBlending,
      .pDynamicState       = &dynamicState,
      .layout              = pipelineLayout,
      .renderPass          = renderPass,
      .subpass             = 0,
      .basePipelineHandle  = VK_NULL_HANDLE,
      .basePipelineIndex   = -1,
  };

  res = vkCreateGraphicsPipelines(
      device,
      VK_NULL_HANDLE,
      1,
      &pipelineInfo,
      nullptr,
      graphicsPipeline.ptr()
  );
  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Failed to create the graphics pipeline !\n\tError value : " << res
       << std::endl;
    throw std::runtime_error(ss.str().c_str());
  }
}

VkResult GraphicsPipeline::CreateRenderPass(
    const SwapchainManager &sm,
    VkDevice                device
)
{
  VkAttachmentDescription colorAttachment{
      .flags          = 0u,
      .format         = sm.getFormat(),
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };
  VkAttachmentReference colorAttachmentRef{
      .attachment = 0,
      .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };
  VkSubpassDescription subpass{
      .flags                   = 0u,
      .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount    = 0u,
      .pInputAttachments       = nullptr,
      .colorAttachmentCount    = 1u,
      .pColorAttachments       = &colorAttachmentRef,
      .pResolveAttachments     = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0u,
      .pPreserveAttachments    = nullptr
  };
  VkRenderPassCreateInfo renderPassInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0u,
      .attachmentCount = 1u,
      .pAttachments    = &colorAttachment,
      .subpassCount    = 1u,
      .pSubpasses      = &subpass,
      .dependencyCount = 0u,
      .pDependencies   = nullptr
  };
  VkResult res =
      vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass.ptr());

  if (res != VK_SUCCESS) {
    std::stringstream ss;
    ss << "Failed to create render pass !\n\tError value : " << res
       << std::endl;
    throw std::runtime_error(ss.str().c_str());
  }

  return res;
}
