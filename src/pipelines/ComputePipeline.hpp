#ifndef COMPUTE_PIPELINE_HPP
#define COMPUTE_PIPELINE_HPP

#include "MemoryWrapper.hpp"
#include "PipelineBase.hpp"
#include "ShaderHandler.hpp"
#include <vulkan/vulkan_core.h>

/**
 * @brief Compute pipeline for general-purpose GPU computation
 * Uses a single compute shader stage
 */
class ComputePipeline : public PipelineBase {
   public:
  /**
   * @brief Construct a compute pipeline
   * @param device Logical device handle
   * @param shaders Range of shader data files (should contain one .comp shader)
   */
  template <std::ranges::range R>
    requires std::same_as<
        std::ranges::range_value_t<R>,
        ShaderDataFile> || std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  ComputePipeline(
      VkDevice device,
      const R &shaders
  )
      : PipelineBase(device, shaders)
  {
    if (std::ranges::empty(shaders)) {
      throw std::runtime_error("ComputePipeline requires at least one shader");
    }

    // Validate that we have a compute shader
    bool hasComputeShader = false;
    for (const auto &shader : shaders) {
      const auto           &reflectModule = shader.getReflectModule();
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
    std::vector<ShaderModuleWrapper> shaderModules;
    shaderModules.reserve(std::ranges::size(shaders));
    VkPipelineShaderStageCreateInfo computeStage{};

    for (const auto &shader : shaders) {
      VkShaderModuleCreateInfo moduleInfo = shader;
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

      computeStage = shader.getPSSCI(shaderModules.back());
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

  ~ComputePipeline() = default;

  DELETE_COPY(ComputePipeline)
  DELETE_MOVE(ComputePipeline)

  /**
   * @brief Get the pipeline bind point (compute)
   */
  [[nodiscard]] VkPipelineBindPoint getBindPoint() const noexcept override
  {
    return VK_PIPELINE_BIND_POINT_COMPUTE;
  }

  /**
   * @brief Dispatch compute work
   * @param commandBuffer Command buffer to record into
   * @param groupCountX Number of work groups in X dimension
   * @param groupCountY Number of work groups in Y dimension
   * @param groupCountZ Number of work groups in Z dimension
   */
  void dispatch(
      VkCommandBuffer commandBuffer,
      uint32_t        groupCountX,
      uint32_t        groupCountY,
      uint32_t        groupCountZ
  ) const noexcept;
};

#endif // COMPUTE_PIPELINE_HPP
