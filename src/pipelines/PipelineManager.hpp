#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include "ComputePipeline.hpp"
#include "GraphicsPipeline.hpp"
#include "PipelineBase.hpp"
#include "RayTracingPipeline.hpp"
#include <deque>
#include <string>
#include <unordered_map>
#include <variant>
#include <vulkan/vulkan_core.h>

/**
 * @brief Manages all pipeline types in a single heterogeneous container
 * Uses std::variant for type-safe storage without heap allocation
 */
class PipelineManager {
  // Constrained variant - only accepts PipelineBase derivatives
  template <typename... Ts>
    requires(std::derived_from<Ts, PipelineBase> && ...)
  using PipelineVariant = std::variant<Ts...>;

  using AllPipelines = PipelineVariant<
      GraphicsPipeline,
      ComputePipeline,
      RayTracingPipeline>;

  VkDevice                                device;
  std::deque<AllPipelines>                pipelines;
  std::unordered_map<std::string, size_t> pipelineIndices;

  // Optional: "main" pipeline references for quick access
  size_t mainGraphicsPipelineIndex = static_cast<size_t>(-1);
  size_t mainComputePipelineIndex  = static_cast<size_t>(-1);

   public:
  PipelineManager(
      VkDevice device
  );
  ~PipelineManager() = default;

  DELETE_COPY(PipelineManager)
  DELETE_MOVE(PipelineManager)

  /**
   * @brief Create a graphics pipeline
   * @param name Unique identifier for the pipeline
   * @param swapchain Swapchain manager for render pass configuration
   * @param shaders Vector of shader data files
   * @param setAsMain If true, sets this as the main graphics pipeline
   * @return Reference to the created pipeline
   */
  GraphicsPipeline &createGraphicsPipeline(
      const std::string                         &name,
      const SwapchainManager                    &swapchain,
      const std::vector<const ShaderDataFile *> &shaders,
      bool                                       setAsMain = false
  );

  /**
   * @brief Create a compute pipeline
   * @param name Unique identifier for the pipeline
   * @param shaders Vector of shader data files
   * @param setAsMain If true, sets this as the main compute pipeline
   * @return Reference to the created pipeline
   */
  ComputePipeline &createComputePipeline(
      const std::string                         &name,
      const std::vector<const ShaderDataFile *> &shaders,
      bool                                       setAsMain = false
  );

  /**
   * @brief Create a ray tracing pipeline
   * @param name Unique identifier for the pipeline
   * @param shaders Vector of shader data files (rgen, rchit, rmiss, etc.)
   * @return Reference to the created pipeline
   */
  RayTracingPipeline &createRayTracingPipeline(
      const std::string                         &name,
      const std::vector<const ShaderDataFile *> &shaders
  );

  /**
   * @brief Get a pipeline by name (returns base type pointer)
   * @param name Pipeline identifier
   * @return Pointer to pipeline base, or nullptr if not found
   */
  PipelineBase *getPipeline(
      const std::string &name
  );

  /**
   * @brief Get a typed pipeline by name
   * @tparam T Pipeline type (GraphicsPipeline, ComputePipeline, etc.)
   * @param name Pipeline identifier
   * @return Pointer to typed pipeline, or nullptr if not found or wrong type
   */
  template <std::derived_from<PipelineBase> T>
  T *getPipelineAs(
      const std::string &name
  )
  {
    auto it = pipelineIndices.find(name);
    if (it == pipelineIndices.end())
      return nullptr;

    return std::get_if<T>(&pipelines[it->second]);
  }

  /**
   * @brief Get the main graphics pipeline
   * @return Pointer to main graphics pipeline, or nullptr if not set
   */
  GraphicsPipeline *getMainGraphicsPipeline();

  /**
   * @brief Get the main compute pipeline
   * @return Pointer to main compute pipeline, or nullptr if not set
   */
  ComputePipeline *getMainComputePipeline();

  /**
   * @brief Bind a pipeline to a command buffer
   * @param name Pipeline identifier
   * @param commandBuffer Command buffer to bind to
   */
  void bindPipeline(
      const std::string &name,
      VkCommandBuffer    commandBuffer
  );

  /**
   * @brief Get total number of pipelines
   */
  [[nodiscard]] size_t getPipelineCount() const noexcept
  {
    return pipelines.size();
  }

  /**
   * @brief Check if a pipeline exists
   */
  [[nodiscard]] bool hasPipeline(
      const std::string &name
  ) const noexcept
  {
    return pipelineIndices.contains(name);
  }
};

#endif // PIPELINE_MANAGER_HPP
