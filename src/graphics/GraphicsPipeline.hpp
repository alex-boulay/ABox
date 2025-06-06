#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "MemoryWrapper.hpp"
#include "SwapchainManager.hpp"
#include <array>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const std::array<VkDynamicState, 2> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};
DEFINE_VK_MEMORY_WRAPPER(
    VkRenderPass,
    RenderPass,
    vkDestroyRenderPass
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

class GraphicsPipeline {
  PipelineWrapper       graphicsPipeline; // main object
  RenderPassWrapper     renderPass;
  PipelineLayoutWrapper pipelineLayout;

  VkViewport viewport;
  VkRect2D   scissor;

  VkResult CreateRenderPass(const SwapchainManager &sm, VkDevice device);

   public:
  GraphicsPipeline(
      const SwapchainManager                      &sm,
      VkDevice                                     device,
      std::vector<VkPipelineShaderStageCreateInfo> shaderStages
  );

  ~GraphicsPipeline() = default;

  [[nodiscard]] VkRenderPass getRenderPass() const noexcept
  {
    return renderPass;
  }
  DELETE_MOVE(
      GraphicsPipeline
  )

  DELETE_COPY(
      GraphicsPipeline
  )
};

#endif
