#include "ComputePipeline.hpp"
#include "PreProcUtils.hpp"
#include "ShaderHandler.hpp"
#include <iostream>
#include <stdexcept>

// ComputePipeline constructor is now templated in the header file

void ComputePipeline::dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t        groupCountX,
    uint32_t        groupCountY,
    uint32_t        groupCountZ
) const noexcept
{
  vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
