#ifndef PIPELINE_BASE_HPP
#define PIPELINE_BASE_HPP

#include "Logger.hpp"
#include "MemoryWrapper.hpp"
#include "ShaderHandler.hpp"
#include <map>
#include <ranges>
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

DEFINE_VK_MEMORY_WRAPPER(VkPipeline, Pipeline, vkDestroyPipeline)

/**
 * @brief Base class for all pipeline types (Graphics, Compute, etc.)
 * Handles common functionality like reflection-based descriptor set layout
 * and pipeline layout creation
 */
class PipelineBase {
   protected:
  std::vector<VkPushConstantRange>        pushConstantRanges;
  std::vector<DescriptorSetLayoutWrapper> descriptorSetLayouts;
  PipelineLayoutWrapper                   pipelineLayout;
  PipelineWrapper                         pipeline;

  /**
   * @brief Build descriptor set layouts and push constant ranges from shader
   * reflection data (internal helper)
   * @param device Logical device handle
   * @param shaders Range of shader data files with reflection information
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  void buildReflectionDataImpl(VkDevice device, const R &shaders)
  {
    std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>
        setBindingsMap;

    for (const ShaderDataFile &shader : shaders) {
      if (!shader.isReflectionValid()) {
        LOG_WARN("Shader") << "Skipping shader with invalid reflection data";
        continue;
      }

      const auto &reflectModule  = shader.getReflectModule();
      const auto &reflectionData = shader.getReflectionData();

      VkShaderStageFlagBits stage =
          static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);

      for (uint32_t i = 0; i < reflectionData.descriptorSetCount; ++i) {
        const SpvReflectDescriptorSet *set = reflectionData.descriptorSets[i];

        LOG_DEBUG("Shader") << "Processing descriptor set " << set->set
                            << " with " << set->binding_count << " bindings";

        for (uint32_t j = 0; j < set->binding_count; ++j) {
          const SpvReflectDescriptorBinding *binding = set->bindings[j];

          VkDescriptorSetLayoutBinding layoutBinding{};
          layoutBinding.binding = binding->binding;
          layoutBinding.descriptorType =
              static_cast<VkDescriptorType>(binding->descriptor_type);
          layoutBinding.descriptorCount    = binding->count;
          layoutBinding.stageFlags         = stage;
          layoutBinding.pImmutableSamplers = nullptr;

          auto &bindings        = setBindingsMap[set->set];
          auto  existingBinding = std::find_if(
              bindings.begin(),
              bindings.end(),
              [&](const VkDescriptorSetLayoutBinding &b) {
                return b.binding == layoutBinding.binding;
              }
          );

          if (existingBinding != bindings.end()) {
            existingBinding->stageFlags |= stage;
            LOG_DEBUG("Shader")
                << "  Merged binding " << layoutBinding.binding
                << " with existing (stages: 0x" << std::hex
                << existingBinding->stageFlags << std::dec << ")";
          }
          else {
            bindings.push_back(layoutBinding);
            LOG_DEBUG("Shader") << "  Added binding " << layoutBinding.binding
                                << ", type: " << layoutBinding.descriptorType
                                << ", count: " << layoutBinding.descriptorCount;
          }
        }
      }

      for (uint32_t i = 0; i < reflectionData.pushConstantCount; ++i) {
        const SpvReflectBlockVariable *pushConstant =
            reflectionData.pushConstants[i];
        VkPushConstantRange range{};
        range.stageFlags   = stage;
        range.offset       = pushConstant->offset;
        range.size         = pushConstant->size;
        auto existingRange = std::find_if(
            pushConstantRanges.begin(),
            pushConstantRanges.end(),
            [&](const VkPushConstantRange &r) {
              return r.offset == range.offset && r.size == range.size;
            }
        );
        if (existingRange != pushConstantRanges.end()) {
          existingRange->stageFlags |= stage;
          LOG_DEBUG("Shader")
              << "Merged push constant range (offset: " << range.offset
              << ", size: " << range.size << ", stages: 0x" << std::hex
              << existingRange->stageFlags << std::dec << ")";
        }
        else {
          pushConstantRanges.push_back(range);
          LOG_DEBUG("Shader")
              << "Added push constant range (offset: " << range.offset
              << ", size: " << range.size << ", stage: 0x" << std::hex
              << range.stageFlags << std::dec << ")";
        }
      }
    }

    descriptorSetLayouts.reserve(setBindingsMap.size());
    for (const auto &[setIndex, bindings] : setBindingsMap) {
      VkDescriptorSetLayoutCreateInfo layoutInfo{
          .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
          .pNext        = nullptr,
          .flags        = 0,
          .bindingCount = static_cast<uint32_t>(bindings.size()),
          .pBindings    = bindings.data()
      };
      while (descriptorSetLayouts.size() <= setIndex) {
        descriptorSetLayouts.emplace_back(device);
      }

      VkResult result = vkCreateDescriptorSetLayout(
          device,
          &layoutInfo,
          nullptr,
          descriptorSetLayouts[setIndex].ptr()
      );

      if (result != VK_SUCCESS) {
        throw std::runtime_error(
            "Failed to create descriptor set layout for set " +
            std::to_string(setIndex)
        );
      }

      LOG_DEBUG("Shader") << "Created descriptor set layout for set "
                          << setIndex << " with " << bindings.size()
                          << " bindings";
    }
  }

  /**
   * @brief Create the pipeline layout from descriptor set layouts and push
   * constants
   * @param device Logical device handle
   */
  void createPipelineLayout(VkDevice device);

  /**
   * @brief Helper to convert SpvReflectFormat to string for error messages
   * @param format SPIRV-Reflect format enum
   * @return Human-readable type string
   */
  static const char *formatToString(SpvReflectFormat format);

   public:
  /**
   * @brief Construct pipeline base and build reflection data
   * @param device Logical device handle
   * @param shaders Range of shader data files (accepts any container: list,
   * vector, deque, etc.)
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
                 std::same_as<
                     std::ranges::range_value_t<R>,
                     const ShaderDataFile>
  PipelineBase(VkDevice device, const R &shaders)
      : pipeline(device)
      , pipelineLayout(device)
  {
    buildReflectionDataImpl(device, shaders);
    createPipelineLayout(device);
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
  void
      pushConstants(VkCommandBuffer commandBuffer, const T &data) const noexcept
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
