#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "SwapchainManager.hpp"
#include <array>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const std::array<VkDynamicState, 2> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

class GraphicsPipeline {
  // External element
  VkDevice device;

  VkViewport viewport;
  VkRect2D   scissor;

  VkRenderPass     renderPass;
  VkPipelineLayout pipelineLayout;

  VkPipeline graphicsPipeline; // main object

  // bindShaderHandlers here

  VkResult CreateRenderPass(const SwapchainManager &sm);

   public:
  GraphicsPipeline(
      const SwapchainManager                      &sm,
      VkDevice                                     device,
      std::vector<VkPipelineShaderStageCreateInfo> shaderStages
  );
  ~GraphicsPipeline();

  // Move Constructor
  GraphicsPipeline(GraphicsPipeline &&other) noexcept;

  // Move Assignment Operator
  GraphicsPipeline &operator=(GraphicsPipeline &&other) noexcept;

  [[nodiscard]] VkRenderPass getRenderPass() const noexcept
  {
    return renderPass;
  }

  DELETE_COPY(
      GraphicsPipeline
  )
};

#endif
