#include "GraphicsPipeline.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

class ComputePipeline {
  // Nex two object common for every pipeline
  PipelineWrapper       pipeline;       // main object
  PipelineLayoutWrapper pipelineLayout; // layout

  // -> Load the right queueFamily to compute
  // VkDescriptorSetLayout for each shader Stage ?
  // pushConstantRangeCount for each shader Stage ?
  // how to bind them ?
  //
   public:
  ComputePipeline(
      VkDevice                                     device,
      std::vector<VkPipelineShaderStageCreateInfo> shaderStages
  )
      : pipeline(device)
      , pipelineLayout(device)
  {
    VkResult                   result = VK_SUCCESS;
    VkPipelineLayoutCreateInfo plci{
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = 0u,
        .setLayoutCount = static_cast<uint32_t>(shaderStages.size()),
        .pSetLayouts    = &computeDescriptorLayout, // TODO <- bind to each
        .pushConstantRangeCount =
            static_cast<uint32_t>(shaderStages.size()), // TODO <- bind to each
        .pPushConstantRanges = 0                        // TODO <- bind to each
    };
    result =
        vkCreatePipelineLayout(device, &plci, nullptr, pipelineLayout.ptr());

    if (result != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create pipelineLayout for the compute pipeline !"
      );
    }
    else {
      std::cout
          << " Success creating the pipeline layout for the compute pipeline "
          << std::endl;
    }
    for (const auto &a : shaderStages) {
      VkComputePipelineCreateInfo cpci{
          .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
          .pNext              = nullptr,
          .flags              = 0u,
          .stage              = a,
          .layout             = pipelineLayout,
          .basePipelineHandle = nullptr,
          .basePipelineIndex  = 0
      };
    }
  };
};
