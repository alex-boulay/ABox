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
    PipelineObj,
    vkDestroyPipeline
)

/**
 * @brief Base class for all pipeline types (Graphics, Compute, etc.)
 * Handles common functionality like reflection-based descriptor set layout
 * and pipeline layout creation
 */
class PipelineBase {
   protected:
  VkDevice device;

  PipelineObjWrapper    pipeline;
  PipelineLayoutWrapper pipelineLayout;

  std::vector<DescriptorSetLayoutWrapper> descriptorSetLayouts;
  std::vector<VkPushConstantRange>        pushConstantRanges;

  /**
   * @brief Build descriptor set layouts and push constant ranges from shader
   * reflection data
   * @param shaders Vector of shader data files with reflection information
   */
  void buildReflectionData(const std::vector<const ShaderDataFile *> &shaders);

  /**
   * @brief Create the pipeline layout from descriptor set layouts and push
   * constants
   */
  void createPipelineLayout();

   public:
  PipelineBase(
      VkDevice dev
  )
      : device(dev)
      , pipeline(dev)
      , pipelineLayout(dev)
  {
  }

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
   * @brief Print reflection information for debugging
   */
  void printReflectionInfo() const;
};

#endif // PIPELINE_BASE_HPP
