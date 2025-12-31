#include "PipelineBase.hpp"
#include "Logger.hpp"
#include "PreProcUtils.hpp"
#include <iostream>
#include <spirv_reflect.h>
#include <stdexcept>

// PipelineBase constructor is now templated in the header file

const char *PipelineBase::formatToString(SpvReflectFormat format)
{
  switch (format) {
    case SPV_REFLECT_FORMAT_UNDEFINED: return "undefined";
    case SPV_REFLECT_FORMAT_R32_UINT: return "uint";
    case SPV_REFLECT_FORMAT_R32_SINT: return "int";
    case SPV_REFLECT_FORMAT_R32_SFLOAT: return "float";
    case SPV_REFLECT_FORMAT_R32G32_UINT: return "uvec2";
    case SPV_REFLECT_FORMAT_R32G32_SINT: return "ivec2";
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return "vec2";
    case SPV_REFLECT_FORMAT_R32G32B32_UINT: return "uvec3";
    case SPV_REFLECT_FORMAT_R32G32B32_SINT: return "ivec3";
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return "vec3";
    case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return "uvec4";
    case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return "ivec4";
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return "vec4";
    case SPV_REFLECT_FORMAT_R64_UINT: return "uint64";
    case SPV_REFLECT_FORMAT_R64_SINT: return "int64";
    case SPV_REFLECT_FORMAT_R64_SFLOAT: return "double";
    case SPV_REFLECT_FORMAT_R64G64_UINT: return "u64vec2";
    case SPV_REFLECT_FORMAT_R64G64_SINT: return "i64vec2";
    case SPV_REFLECT_FORMAT_R64G64_SFLOAT: return "dvec2";
    case SPV_REFLECT_FORMAT_R64G64B64_UINT: return "u64vec3";
    case SPV_REFLECT_FORMAT_R64G64B64_SINT: return "i64vec3";
    case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT: return "dvec3";
    case SPV_REFLECT_FORMAT_R64G64B64A64_UINT: return "u64vec4";
    case SPV_REFLECT_FORMAT_R64G64B64A64_SINT: return "i64vec4";
    case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return "dvec4";
    default: return "unknown";
  }
}

void PipelineBase::createPipelineLayout(VkDevice device)
{
  std::vector<VkDescriptorSetLayout> layouts;
  layouts.reserve(descriptorSetLayouts.size());

  for (const auto &layout : descriptorSetLayouts) {
    layouts.push_back(layout.get());
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext          = nullptr,
      .flags          = 0,
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts    = layouts.data(),
      .pushConstantRangeCount =
          static_cast<uint32_t>(pushConstantRanges.size()),
      .pPushConstantRanges = pushConstantRanges.data()
  };

  VkResult result = vkCreatePipelineLayout(
      device,
      &pipelineLayoutInfo,
      nullptr,
      pipelineLayout.ptr()
  );

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout!");
  }

  LOG_DEBUG("Pipeline") << "Created pipeline layout with " << layouts.size()
                        << " descriptor sets and " << pushConstantRanges.size()
                        << " push constant ranges";
}

void PipelineBase::bind(VkCommandBuffer commandBuffer) const noexcept
{
  vkCmdBindPipeline(commandBuffer, getBindPoint(), pipeline);
}

void PipelineBase::bindDescriptorSets(
    VkCommandBuffer                     commandBuffer,
    const std::vector<VkDescriptorSet> &descriptorSets,
    uint32_t                            firstSet
) const noexcept
{
  if (!descriptorSets.empty()) {
    vkCmdBindDescriptorSets(
        commandBuffer,
        getBindPoint(),
        pipelineLayout,
        firstSet,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        0,
        nullptr
    );
  }
}

void PipelineBase::printReflectionInfo() const
{
  LOG_INFO("Pipeline") << "=== Pipeline Reflection Info ===";
  LOG_INFO("Pipeline") << "Descriptor Sets: " << descriptorSetLayouts.size();
  LOG_INFO("Pipeline") << "Push Constant Ranges: " << pushConstantRanges.size();

  for (size_t i = 0; i < pushConstantRanges.size(); ++i) {
    const auto &range = pushConstantRanges[i];
    LOG_DEBUG("Pipeline") << "  Push Constant " << i
                          << ": offset=" << range.offset
                          << ", size=" << range.size << ", stages=0x"
                          << std::hex << range.stageFlags << std::dec;
  }
}
