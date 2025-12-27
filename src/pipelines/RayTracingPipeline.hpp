#ifndef RAY_TRACING_PIPELINE_HPP
#define RAY_TRACING_PIPELINE_HPP

#include "MemoryWrapper.hpp"
#include "PipelineBase.hpp"
#include "ShaderHandler.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

// Forward declarations for ray tracing structures
DEFINE_VK_MEMORY_WRAPPER(
    VkAccelerationStructureKHR,
    AccelerationStructure,
    vkDestroyAccelerationStructureKHR
)

/**
 * @brief Shader Binding Table wrapper for ray tracing
 * Contains shader group handles for ray generation, miss, hit, and callable
 * shaders
 */
struct ShaderBindingTable {
  VkBuffer               buffer       = VK_NULL_HANDLE;
  VkDeviceMemory         memory       = VK_NULL_HANDLE;
  VkStridedDeviceAddressRegionKHR raygenRegion   = {};
  VkStridedDeviceAddressRegionKHR missRegion     = {};
  VkStridedDeviceAddressRegionKHR hitRegion      = {};
  VkStridedDeviceAddressRegionKHR callableRegion = {};

  // TODO: Add proper cleanup when implemented
  void destroy(
      VkDevice device
  );
};

/**
 * @brief Ray tracing pipeline for hardware-accelerated ray tracing
 * Supports ray generation, closest hit, any hit, miss, intersection, and
 * callable shaders
 *
 * NOTE: This is a blueprint implementation. Full ray tracing support requires:
 * - VK_KHR_ray_tracing_pipeline extension
 * - VK_KHR_acceleration_structure extension
 * - Acceleration structure creation (BLAS/TLAS)
 * - Shader binding table (SBT) setup
 */
class RayTracingPipeline : public PipelineBase {
  ShaderBindingTable sbt;

  // Ray tracing specific shader groups
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

  /**
   * @brief Build shader groups from shader stages
   * Groups shaders into ray gen, miss, hit groups, etc.
   */
  void buildShaderGroups(
      const std::vector<const ShaderDataFile *> &shaders
  );

  /**
   * @brief Create the shader binding table
   * Allocates and fills the SBT with shader group handles
   */
  void createShaderBindingTable(
      VkDevice device
  );

   public:
  /**
   * @brief Construct a ray tracing pipeline
   * @param device Logical device handle
   * @param shaders Vector of ray tracing shaders (.rgen, .rchit, .rmiss, etc.)
   *
   * NOTE: Currently a stub implementation. Requires:
   * - Extension support checking
   * - Acceleration structure integration
   * - SBT creation
   */
  RayTracingPipeline(
      VkDevice                                   device,
      const std::vector<const ShaderDataFile *> &shaders
  );

  ~RayTracingPipeline();

  DELETE_COPY(RayTracingPipeline)
  DELETE_MOVE(RayTracingPipeline)

  /**
   * @brief Get the pipeline bind point (ray tracing)
   */
  [[nodiscard]] VkPipelineBindPoint getBindPoint() const noexcept override
  {
    return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
  }

  /**
   * @brief Trace rays using this pipeline
   * @param commandBuffer Command buffer to record into
   * @param width Ray trace dispatch width
   * @param height Ray trace dispatch height
   * @param depth Ray trace dispatch depth (default 1)
   *
   * NOTE: Stub implementation - needs SBT to be fully implemented
   */
  void traceRays(
      VkCommandBuffer commandBuffer,
      uint32_t        width,
      uint32_t        height,
      uint32_t        depth = 1
  ) const;

  /**
   * @brief Get the shader binding table
   */
  [[nodiscard]] const ShaderBindingTable &getSBT() const noexcept
  {
    return sbt;
  }
};

#endif // RAY_TRACING_PIPELINE_HPP
