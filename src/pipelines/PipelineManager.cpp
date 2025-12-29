#include "PipelineManager.hpp"
#include "PreProcUtils.hpp"

PipelineManager::PipelineManager()
{
  FILE_DEBUG_PRINT("PipelineManager created");
}

// Pipeline creation functions are now templated in the header file

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
