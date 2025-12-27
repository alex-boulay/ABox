#include "ComputePipeline.hpp"
#include "PreProcUtils.hpp"
#include <iostream>
#include <stdexcept>

ComputePipeline::ComputePipeline(
    VkDevice                                   device,
    const std::vector<const ShaderDataFile *> &shaders
)
    : PipelineBase(device, shaders)
{
  if (shaders.empty()) {
    throw std::runtime_error("ComputePipeline requires at least one shader");
  }

  // Validate that we have a compute shader
  bool hasComputeShader = false;
  for (const auto *shader : shaders) {
    const auto           &reflectModule = shader->getReflectModule();
    VkShaderStageFlagBits stage =
        static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
      hasComputeShader = true;
      break;
    }
  }

  if (!hasComputeShader) {
    throw std::runtime_error(
        "ComputePipeline requires at least one compute shader (.comp)"
    );
  }

  // Create shader modules and stage info
  std::vector<ShaderModule> shaderModules;
  shaderModules.reserve(shaders.size());
  VkPipelineShaderStageCreateInfo computeStage{};

  for (const auto *shader : shaders) {
    VkShaderModuleCreateInfo moduleInfo = *shader;
    shaderModules.emplace_back(device);

    VkResult result = vkCreateShaderModule(
        device,
        &moduleInfo,
        nullptr,
        shaderModules.back().ptr()
    );

    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create compute shader module");
    }

    computeStage = shader->getPSSCI(shaderModules.back());
  }

  // Create compute pipeline
  VkComputePipelineCreateInfo pipelineInfo{
      .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext              = nullptr,
      .flags              = 0u,
      .stage              = computeStage,
      .layout             = pipelineLayout,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex  = -1
  };

  VkResult result = vkCreateComputePipelines(
      device,
      VK_NULL_HANDLE,
      1,
      &pipelineInfo,
      nullptr,
      pipeline.ptr()
  );

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create compute pipeline!");
  }

  FILE_DEBUG_PRINT("ComputePipeline created successfully");
  printReflectionInfo();
}

void ComputePipeline::dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t        groupCountX,
    uint32_t        groupCountY,
    uint32_t        groupCountZ
) const noexcept
{
  vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
