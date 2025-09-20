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
  DELETE_MOVE(
      GraphicsPipeline
  )

  DELETE_COPY(
      GraphicsPipeline
  )

  GraphicsPipeline(
      const SwapchainManager                      &sm,
      VkDevice                                     device,
      std::vector<VkPipelineShaderStageCreateInfo> shaderStages
  );

  ~GraphicsPipeline() = default;

  [[nodiscard]] VkPipeline getPipeline() const noexcept
  {
    return graphicsPipeline;
  }

  [[nodiscard]] VkRenderPass getRenderPass() const noexcept
  {
    return renderPass;
  }

  [[nodiscard]] VkPipelineLayout getPipelineLayout() const noexcept
  {
    return pipelineLayout;
  }

  [[nodiscard]] VkViewport getViewport() const noexcept { return viewport; }

  [[nodiscard]] const VkViewport *getViewportPtr() const noexcept
  {
    return &viewport;
  }

  [[nodiscard]] VkRect2D getScissor() const noexcept { return scissor; }

  [[nodiscard]] const VkRect2D *getScissorPtr() const noexcept
  {
    return &scissor;
  }

  void updateExtent(
      VkExtent2D ext
  )
  {
    std::cout << "Updating extend in GP width :" << ext.width << " - height "
              << ext.height << std::endl;
    scissor = {
        .offset = {0u, 0u},
        .extent = ext
    };
    viewport = {
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = static_cast<float>(scissor.extent.width),
        .height   = static_cast<float>(scissor.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
  }
};

#endif
