#include "PipelineBase.hpp"
#include "PreProcUtils.hpp"
#include <iostream>
#include <spirv_reflect.h>
#include <stdexcept>

PipelineBase::PipelineBase(
    VkDevice                                   device,
    const std::vector<const ShaderDataFile *> &shaders
)
    : pipeline(device)
    , pipelineLayout(device)
{
  buildReflectionData(device, shaders);
  createPipelineLayout(device);
}

void PipelineBase::buildReflectionData(
    VkDevice                                   device,
    const std::vector<const ShaderDataFile *> &shaders
)
{
  std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setBindingsMap;

  for (const ShaderDataFile *shader : shaders) {
    if (!shader || !shader->isReflectionValid()) {
      FILE_DEBUG_PRINT("Skipping shader with invalid reflection data");
      continue;
    }

    const auto &reflectModule  = shader->getReflectModule();
    const auto &reflectionData = shader->getReflectionData();

    VkShaderStageFlagBits stage =
        static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);

    for (uint32_t i = 0; i < reflectionData.descriptorSetCount; ++i) {
      const SpvReflectDescriptorSet *set = reflectionData.descriptorSets[i];

      FILE_DEBUG_PRINT(
          "Processing descriptor set %u with %u bindings",
          set->set,
          set->binding_count
      );

      for (uint32_t j = 0; j < set->binding_count; ++j) {
        const SpvReflectDescriptorBinding *binding = set->bindings[j];

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding->binding;
        layoutBinding.descriptorType =
            static_cast<VkDescriptorType>(binding->descriptor_type);
        layoutBinding.descriptorCount    = binding->count;
        layoutBinding.stageFlags         = stage;
        layoutBinding.pImmutableSamplers = nullptr;

        // Check if this binding already exists in this set from another shader
        // stage
        auto &bindings        = setBindingsMap[set->set];
        auto  existingBinding = std::find_if(
            bindings.begin(),
            bindings.end(),
            [&](const VkDescriptorSetLayoutBinding &b) {
              return b.binding == layoutBinding.binding;
            }
        );

        if (existingBinding != bindings.end()) {
          // Merge stage flags
          existingBinding->stageFlags |= stage;
          FILE_DEBUG_PRINT(
              "  Merged binding %u with existing (stages: %x)",
              layoutBinding.binding,
              existingBinding->stageFlags
          );
        }
        else {
          bindings.push_back(layoutBinding);
          FILE_DEBUG_PRINT(
              "  Added binding %u, type: %d, count: %u",
              layoutBinding.binding,
              layoutBinding.descriptorType,
              layoutBinding.descriptorCount
          );
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
        FILE_DEBUG_PRINT(
            "Merged push constant range (offset: %u, size: %u, stages: %x)",
            range.offset,
            range.size,
            existingRange->stageFlags
        );
      }
      else {
        pushConstantRanges.push_back(range);
        FILE_DEBUG_PRINT(
            "Added push constant range (offset: %u, size: %u, stage: %x)",
            range.offset,
            range.size,
            range.stageFlags
        );
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

    FILE_DEBUG_PRINT(
        "Created descriptor set layout for set %u with %zu bindings",
        setIndex,
        bindings.size()
    );
  }
}

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
