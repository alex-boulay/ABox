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
   * @param shaders Vector of shader data files (should contain one .comp
   * shader)
   */
  ComputePipeline(
      VkDevice                                   device,
      const std::vector<const ShaderDataFile *> &shaders
  );

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
