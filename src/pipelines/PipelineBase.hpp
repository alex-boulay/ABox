#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include "MemoryWrapper.hpp"
#include "ShaderHandler.hpp"
#include <map>
#include <vector>
#include <vulkan/vulkan_core.h>

DEFINE_VK_MEMORY_WRAPPER(
    VkDescriptorSetLayout,
    DescriptorSetLayout,
    vkDestroyDescriptorSetLayout
)

DEFINE_VK_MEMORY_WRAPPER(
    VkPipelineLayout,
    PipelineLayout,
    vkDestroyPipelineLayout
)

DEFINE_VK_MEMORY_WRAPPER(
    VkPipeline,
    Pipeline,
    vkDestroyPipeline
)

/**
 * @brief Base class for all pipeline types (Graphics, Compute, etc.)
 * Handles common functionality like reflection-based descriptor set layout
 * and pipeline layout creation
 */
class PipelineBase {
   protected:
  PipelineWrapper       pipeline;
  PipelineLayoutWrapper pipelineLayout;

  std::vector<DescriptorSetLayoutWrapper> descriptorSetLayouts;
  std::vector<VkPushConstantRange>        pushConstantRanges;

  /**
   * @brief Build descriptor set layouts and push constant ranges from shader
   * reflection data
   * @param device Logical device handle
   * @param shaders Vector of shader data files with reflection information
   */
  void buildReflectionData(
      VkDevice                                   device,
      const std::vector<const ShaderDataFile *> &shaders
  );

  /**
   * @brief Create the pipeline layout from descriptor set layouts and push
   * constants
   * @param device Logical device handle
   */
  void createPipelineLayout(VkDevice device);

   public:
  /**
   * @brief Construct pipeline base and build reflection data
   * @param device Logical device handle
   * @param shaders Vector of shader data files with reflection information
   */
  PipelineBase(
      VkDevice                                   device,
      const std::vector<const ShaderDataFile *> &shaders
  );

  virtual ~PipelineBase() = default;

  DELETE_COPY(PipelineBase);
  DELETE_MOVE(PipelineBase);

  [[nodiscard]] VkPipeline getPipeline() const noexcept { return pipeline; }

  [[nodiscard]] VkPipelineLayout getPipelineLayout() const noexcept
  {
    return pipelineLayout;
  }

  [[nodiscard]] const std::vector<DescriptorSetLayoutWrapper> &
      getDescriptorSetLayouts() const noexcept
  {
    return descriptorSetLayouts;
  }

  [[nodiscard]] const std::vector<VkPushConstantRange> &
      getPushConstantRanges() const noexcept
  {
    return pushConstantRanges;
  }

  /**
   * @brief Get the pipeline bind point (graphics or compute)
   * @return VkPipelineBindPoint for this pipeline type
   */
  [[nodiscard]] virtual VkPipelineBindPoint getBindPoint() const noexcept = 0;

  /**
   * @brief Bind this pipeline to a command buffer
   * @param commandBuffer Command buffer to bind to
   */
  void bind(VkCommandBuffer commandBuffer) const noexcept;

  /**
   * @brief Bind descriptor sets for this pipeline
   * @param commandBuffer Command buffer to bind to
   * @param descriptorSets Descriptor sets to bind
   * @param firstSet First descriptor set index (default 0)
   */
  void bindDescriptorSets(
      VkCommandBuffer                     commandBuffer,
      const std::vector<VkDescriptorSet> &descriptorSets,
      uint32_t                            firstSet = 0
  ) const noexcept;

  /**
   * @brief Push constants for this pipeline (type-safe)
   * @tparam T Type of push constant data
   * @param commandBuffer Command buffer to push to
   * @param data Push constant data (validates size at compile time)
   */
  template <typename T>
  void pushConstants(
      VkCommandBuffer commandBuffer,
      const T        &data
  ) const noexcept
  {
    static_assert(
        sizeof(T) <= 128,
        "Push constants limited to 128 bytes in most implementations"
    );

    for (const auto &range : pushConstantRanges) {
      vkCmdPushConstants(
          commandBuffer,
          pipelineLayout,
          range.stageFlags,
          range.offset,
          range.size,
          reinterpret_cast<const uint8_t *>(&data) + range.offset
      );
    }
  }

  /**
   * @brief Print reflection information for debugging
   */
  void printReflectionInfo() const;
};

#endif // PIPELINE_BASE_HPP
