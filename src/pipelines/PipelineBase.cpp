#include "PipelineBase.hpp"
#include "PreProcUtils.hpp"
#include <iostream>
#include <spirv_reflect.h>
#include <stdexcept>

// PipelineBase constructor is now templated in the header file

void PipelineBase::createPipelineLayout(
    VkDevice device
)
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

  FILE_DEBUG_PRINT(
      "Created pipeline layout with %zu descriptor sets and %zu push constant "
      "ranges",
      layouts.size(),
      pushConstantRanges.size()
  );
}

void PipelineBase::bind(
    VkCommandBuffer commandBuffer
) const noexcept
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
  std::cout << "=== Pipeline Reflection Info ===" << std::endl;
  std::cout << "Descriptor Sets: " << descriptorSetLayouts.size() << std::endl;
  std::cout << "Push Constant Ranges: " << pushConstantRanges.size()
            << std::endl;

  for (size_t i = 0; i < pushConstantRanges.size(); ++i) {
    const auto &range = pushConstantRanges[i];
    std::cout << "  Push Constant " << i << ": offset=" << range.offset
              << ", size=" << range.size << ", stages=0x" << std::hex
              << range.stageFlags << std::dec << std::endl;
  }
}
