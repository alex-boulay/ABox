#ifndef RAY_TRACING_PIPELINE_HPP
#define RAY_TRACING_PIPELINE_HPP

#include "Logger.hpp"
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
  VkBuffer                        buffer         = VK_NULL_HANDLE;
  VkDeviceMemory                  memory         = VK_NULL_HANDLE;
  VkStridedDeviceAddressRegionKHR raygenRegion   = {};
  VkStridedDeviceAddressRegionKHR missRegion     = {};
  VkStridedDeviceAddressRegionKHR hitRegion      = {};
  VkStridedDeviceAddressRegionKHR callableRegion = {};

  // TODO: Add proper cleanup when implemented
  void destroy(VkDevice device);
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
   * @brief Validate that required shader stages are present for ray tracing
   * pipeline
   * @param shaders Range of shader data files
   * @throws std::runtime_error if required stages are missing
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  static void validateRayTracingShaderStages(const R &shaders)
  {
    bool hasRayGen = false;

    for (const auto &shader : shaders) {
      const auto           &reflectModule = shader.getReflectModule();
      VkShaderStageFlagBits stage =
          static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
      if (stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR) {
        hasRayGen = true;
        break;
      }
    }

    if (!hasRayGen) {
      throw std::runtime_error("RayTracingPipeline requires at least one ray "
                               "generation shader (.rgen)");
    }
  }

  /**
   * @brief Build shader groups from shader stages
   * Groups shaders into ray gen, miss, hit groups, etc.
   */
  template <std::ranges::range R> void buildShaderGroupsImpl(const R &shaders)
  {
    // Group shaders by type
    // Ray gen shaders -> one group each
    // Miss shaders -> one group each
    // Hit groups -> combine closest hit + any hit + intersection

    for (const auto &shader : shaders) {
      const auto           &reflectModule = shader.getReflectModule();
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

    LOG_DEBUG("Pipeline") << "Built " << shaderGroups.size()
                          << " shader groups";
  }

  /**
   * @brief Create the shader binding table
   * Allocates and fills the SBT with shader group handles
   */
  void createShaderBindingTable(VkDevice device);

   public:
  /**
   * @brief Construct a ray tracing pipeline
   * @param device Logical device handle
   * @param shaders Range of ray tracing shaders (.rgen, .rchit, .rmiss, etc.)
   *
   * NOTE: Currently a stub implementation. Requires:
   * - Extension support checking
   * - Acceleration structure integration
   * - SBT creation
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  RayTracingPipeline(VkDevice device, const R &shaders)
      : PipelineBase(device, shaders)
  {
    LOG_DEBUG("Pipeline") << "RayTracingPipeline construction started with "
                          << std::ranges::size(shaders) << " shaders";

    validateRayTracingShaderStages(shaders);

    // TODO: Check for ray tracing extension support
    // VkPhysicalDeviceRayTracingPipelinePropertiesKHR
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR

    buildShaderGroupsImpl(shaders);

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

    LOG_INFO("Pipeline"
    ) << "RayTracingPipeline construction complete (STUB IMPLEMENTATION)";
    LOG_WARN("Pipeline"
    ) << "WARNING: RayTracingPipeline is a stub - full implementation "
      << "requires extension support";
  }

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
