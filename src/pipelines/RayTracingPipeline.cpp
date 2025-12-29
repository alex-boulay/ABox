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

// RayTracingPipeline constructor is now templated in the header file

RayTracingPipeline::~RayTracingPipeline()
{
  // SBT cleanup handled by wrapper destructor when implemented
  FILE_DEBUG_PRINT("RayTracingPipeline destroyed");
}

void RayTracingPipeline::createShaderBindingTable(
    [[maybe_unused]] VkDevice device
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
    [[maybe_unused]] VkCommandBuffer commandBuffer,
    uint32_t                         width,
    uint32_t                         height,
    uint32_t                         depth
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
