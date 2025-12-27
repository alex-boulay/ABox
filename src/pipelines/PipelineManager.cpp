#include "PipelineManager.hpp"
#include "PreProcUtils.hpp"
#include <iostream>

PipelineManager::PipelineManager(
    VkDevice device
)
    : device(device)
{
  FILE_DEBUG_PRINT("PipelineManager created for device: %p", (void *)device);
}

GraphicsPipeline &PipelineManager::createGraphicsPipeline(
    const std::string                         &name,
    const SwapchainManager                    &swapchain,
    const std::vector<const ShaderDataFile *> &shaders,
    bool                                       setAsMain
)
{
  if (pipelineIndices.contains(name)) {
    FILE_DEBUG_PRINT(
        "Warning: Pipeline '%s' already exists, overwriting",
        name.c_str()
    );
  }

  // Build shader stages
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  std::vector<ShaderModule>                    shaderModules;
  shaderModules.reserve(shaders.size());

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
      throw std::runtime_error(
          "Failed to create shader module for graphics pipeline: " + name
      );
    }

    shaderStages.push_back(shader->getPSSCI(shaderModules.back()));
  }

  size_t index            = pipelines.size();
  pipelineIndices[name]   = index;
  auto &variant           = pipelines.emplace_back(
      std::in_place_type<GraphicsPipeline>,
      swapchain,
      device,
      shaderStages
  );

  if (setAsMain) {
    mainGraphicsPipelineIndex = index;
    FILE_DEBUG_PRINT("Set '%s' as main graphics pipeline", name.c_str());
  }

  FILE_DEBUG_PRINT("Created graphics pipeline: %s", name.c_str());
  return std::get<GraphicsPipeline>(variant);
}

ComputePipeline &PipelineManager::createComputePipeline(
    const std::string                         &name,
    const std::vector<const ShaderDataFile *> &shaders,
    bool                                       setAsMain
)
{
  if (pipelineIndices.contains(name)) {
    FILE_DEBUG_PRINT(
        "Warning: Pipeline '%s' already exists, overwriting",
        name.c_str()
    );
  }

  size_t index          = pipelines.size();
  pipelineIndices[name] = index;
  auto  &variant        = pipelines.emplace_back(
      std::in_place_type<ComputePipeline>,
      device,
      shaders
  );

  if (setAsMain) {
    mainComputePipelineIndex = index;
    FILE_DEBUG_PRINT("Set '%s' as main compute pipeline", name.c_str());
  }

  FILE_DEBUG_PRINT("Created compute pipeline: %s", name.c_str());
  return std::get<ComputePipeline>(variant);
}

RayTracingPipeline &PipelineManager::createRayTracingPipeline(
    const std::string                         &name,
    const std::vector<const ShaderDataFile *> &shaders
)
{
  if (pipelineIndices.contains(name)) {
    FILE_DEBUG_PRINT(
        "Warning: Pipeline '%s' already exists, overwriting",
        name.c_str()
    );
  }

  size_t index          = pipelines.size();
  pipelineIndices[name] = index;
  auto  &variant        = pipelines.emplace_back(
      std::in_place_type<RayTracingPipeline>,
      device,
      shaders
  );

  FILE_DEBUG_PRINT("Created ray tracing pipeline: %s", name.c_str());
  return std::get<RayTracingPipeline>(variant);
}

PipelineBase *PipelineManager::getPipeline(
    const std::string &name
)
{
  auto it = pipelineIndices.find(name);
  if (it == pipelineIndices.end()) {
    FILE_DEBUG_PRINT("Pipeline '%s' not found", name.c_str());
    return nullptr;
  }

  return std::visit(
      [](auto &pipeline) -> PipelineBase * { return &pipeline; },
      pipelines[it->second]
  );
}

GraphicsPipeline *PipelineManager::getMainGraphicsPipeline()
{
  if (mainGraphicsPipelineIndex == static_cast<size_t>(-1)) {
    return nullptr;
  }

  return std::get_if<GraphicsPipeline>(&pipelines[mainGraphicsPipelineIndex]);
}

ComputePipeline *PipelineManager::getMainComputePipeline()
{
  if (mainComputePipelineIndex == static_cast<size_t>(-1)) {
    return nullptr;
  }

  return std::get_if<ComputePipeline>(&pipelines[mainComputePipelineIndex]);
}

void PipelineManager::bindPipeline(
    const std::string &name,
    VkCommandBuffer    commandBuffer
)
{
  auto it = pipelineIndices.find(name);
  if (it == pipelineIndices.end()) {
    FILE_DEBUG_PRINT("Cannot bind: Pipeline '%s' not found", name.c_str());
    return;
  }

  std::visit(
      [commandBuffer](auto &pipeline) { pipeline.bind(commandBuffer); },
      pipelines[it->second]
  );
}
