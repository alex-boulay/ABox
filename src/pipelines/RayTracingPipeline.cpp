#include "RayTracingPipeline.hpp"
#include "PreProcUtils.hpp"
#include <iostream>
#include <stdexcept>

void ShaderBindingTable::destroy(
    VkDevice device
)
{
  if (buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, buffer, nullptr);
    buffer = VK_NULL_HANDLE;
  }
  if (memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, memory, nullptr);
    memory = VK_NULL_HANDLE;
  }
}

RayTracingPipeline::RayTracingPipeline(
    VkDevice                                   device,
    const std::vector<const ShaderDataFile *> &shaders
)
    : PipelineBase(device, shaders)
{
  FILE_DEBUG_PRINT(
      "RayTracingPipeline construction started with %zu shaders",
      shaders.size()
  );

  // TODO: Check for ray tracing extension support
  // VkPhysicalDeviceRayTracingPipelinePropertiesKHR
  // VkPhysicalDeviceAccelerationStructureFeaturesKHR

  buildShaderGroups(shaders);

  // TODO: Create ray tracing pipeline
  // VkRayTracingPipelineCreateInfoKHR pipelineInfo{
  //     .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
  //     .stageCount = shaderStages.size(),
  //     .pStages = shaderStages.data(),
  //     .groupCount = shaderGroups.size(),
  //     .pGroups = shaderGroups.data(),
  //     .maxPipelineRayRecursionDepth = 1,
  //     .layout = pipelineLayout
  // };
  //
  // vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
  //                                &pipelineInfo, nullptr, pipeline.ptr());

  // createShaderBindingTable(device);

  FILE_DEBUG_PRINT(
      "RayTracingPipeline construction complete (STUB IMPLEMENTATION)"
  );
  std::cout << "WARNING: RayTracingPipeline is a stub - full implementation "
               "requires extension support"
            << std::endl;
}

RayTracingPipeline::~RayTracingPipeline()
{
  // SBT cleanup handled by wrapper destructor when implemented
  FILE_DEBUG_PRINT("RayTracingPipeline destroyed");
}

void RayTracingPipeline::buildShaderGroups(
    const std::vector<const ShaderDataFile *> &shaders
)
{
  // Group shaders by type
  // Ray gen shaders -> one group each
  // Miss shaders -> one group each
  // Hit groups -> combine closest hit + any hit + intersection

  for (const auto *shader : shaders) {
    const auto &reflectModule = shader->getReflectModule();
    VkShaderStageFlagBits stage =
        static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);

    VkRayTracingShaderGroupCreateInfoKHR group{};
    group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group.generalShader      = VK_SHADER_UNUSED_KHR;
    group.closestHitShader   = VK_SHADER_UNUSED_KHR;
    group.anyHitShader       = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;

    // TODO: Properly categorize and group shaders
    // This is a simplified stub
    if (stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR ||
        stage == VK_SHADER_STAGE_MISS_BIT_KHR ||
        stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
      group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
      group.generalShader = static_cast<uint32_t>(shaderGroups.size());
    }
    else {
      group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
      // Assign to appropriate hit shader slot
    }

    shaderGroups.push_back(group);
  }

  FILE_DEBUG_PRINT("Built %zu shader groups", shaderGroups.size());
}

void RayTracingPipeline::createShaderBindingTable(
    VkDevice device
)
{
  // TODO: Implement SBT creation
  // 1. Query shader group handles from pipeline
  // 2. Allocate buffer with proper alignment
  // 3. Copy shader handles to buffer
  // 4. Set up VkStridedDeviceAddressRegionKHR for each shader type

  FILE_DEBUG_PRINT("Shader Binding Table creation (STUB)");
}

void RayTracingPipeline::traceRays(
    VkCommandBuffer commandBuffer,
    uint32_t        width,
    uint32_t        height,
    uint32_t        depth
) const
{
  // TODO: Implement when SBT is ready
  // vkCmdTraceRaysKHR(
  //     commandBuffer,
  //     &sbt.raygenRegion,
  //     &sbt.missRegion,
  //     &sbt.hitRegion,
  //     &sbt.callableRegion,
  //     width,
  //     height,
  //     depth
  // );

  FILE_DEBUG_PRINT(
      "RayTracingPipeline::traceRays called (%ux%ux%u) - STUB",
      width,
      height,
      depth
  );
  std::cout << "WARNING: traceRays is not implemented yet" << std::endl;
}
