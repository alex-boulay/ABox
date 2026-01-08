#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include "ComputePipeline.hpp"
#include "GraphicsPipeline.hpp"
#include "Logger.hpp"
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

  using AllPipelines =
      PipelineVariant<GraphicsPipeline, ComputePipeline, RayTracingPipeline>;

  std::deque<AllPipelines>                pipelines;
  std::unordered_map<std::string, size_t> pipelineIndices;

  // Optional: "main" pipeline references for quick access
  size_t mainGraphicsPipelineIndex = static_cast<size_t>(-1);
  size_t mainComputePipelineIndex  = static_cast<size_t>(-1);

   public:
  PipelineManager();
  ~PipelineManager() = default;

  DELETE_COPY(PipelineManager)
  DELETE_MOVE(PipelineManager)

  /**
   * @brief Create a graphics pipeline
   * @param name Unique identifier for the pipeline
   * @param swapchain Swapchain manager for render pass configuration
   * @param shaders Range of shader data files (accepts any container: list,
   * vector, etc.)
   * @param setAsMain If true, sets this as the main graphics pipeline
   * @return Reference to the created pipeline
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  GraphicsPipeline &createGraphicsPipeline(
      VkDevice           device,
      const std::string &name,
      const Swapchain   &swapchain,
      const R           &shaders,
      bool               setAsMain = false
  )
  {
    if (std::ranges::empty(shaders)) {
      LOG_ERROR("Pipeline") << "Cannot create graphics pipeline '" << name
                            << "': no shaders provided";
      throw std::runtime_error(
          "Cannot create graphics pipeline with empty shader list"
      );
    }

    if (pipelineIndices.contains(name)) {
      LOG_WARN("Pipeline") << "Pipeline '" << name
                           << "' already exists, overwriting";
    }

    size_t index          = pipelines.size();
    pipelineIndices[name] = index;

    // GraphicsPipeline constructor handles shader module creation internally
    auto &variant = pipelines.emplace_back(
        std::in_place_type<GraphicsPipeline>,
        device,
        swapchain,
        shaders
    );

    if (setAsMain) {
      mainGraphicsPipelineIndex = index;
      LOG_DEBUG("Pipeline") << "Set '" << name << "' as main graphics pipeline";
    }

    LOG_INFO("Pipeline") << "Created graphics pipeline: " << name;
    return std::get<GraphicsPipeline>(variant);
  }

  /**
   * @brief Create a compute pipeline
   * @param name Unique identifier for the pipeline
   * @param shaders Range of shader data files (accepts any container)
   * @param setAsMain If true, sets this as the main compute pipeline
   * @return Reference to the created pipeline
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  ComputePipeline &createComputePipeline(
      VkDevice           device,
      const std::string &name,
      const R           &shaders,
      bool               setAsMain = false
  )
  {
    if (std::ranges::empty(shaders)) {
      LOG_ERROR("Pipeline") << "Cannot create compute pipeline '" << name
                            << "': no shaders provided";
      throw std::runtime_error(
          "Cannot create compute pipeline with empty shader list"
      );
    }

    if (pipelineIndices.contains(name)) {
      LOG_WARN("Pipeline") << "Pipeline '" << name
                           << "' already exists, overwriting";
    }

    size_t index          = pipelines.size();
    pipelineIndices[name] = index;
    auto &variant         = pipelines.emplace_back(
        std::in_place_type<ComputePipeline>,
        device,
        shaders
    );

    if (setAsMain) {
      mainComputePipelineIndex = index;
      LOG_DEBUG("Pipeline") << "Set '" << name << "' as main compute pipeline";
    }

    LOG_INFO("Pipeline") << "Created compute pipeline: " << name;
    return std::get<ComputePipeline>(variant);
  }

  /**
   * @brief Create a ray tracing pipeline
   * @param name Unique identifier for the pipeline
   * @param shaders Range of shader data files (rgen, rchit, rmiss, etc.)
   * @return Reference to the created pipeline
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  RayTracingPipeline &createRayTracingPipeline(
      VkDevice           device,
      const std::string &name,
      const R           &shaders
  )
  {
    if (std::ranges::empty(shaders)) {
      LOG_ERROR("Pipeline") << "Cannot create ray tracing pipeline '" << name
                            << "': no shaders provided";
      throw std::runtime_error(
          "Cannot create ray tracing pipeline with empty shader list"
      );
    }

    if (pipelineIndices.contains(name)) {
      LOG_WARN("Pipeline") << "Pipeline '" << name
                           << "' already exists, overwriting";
    }

    size_t index          = pipelines.size();
    pipelineIndices[name] = index;
    auto &variant         = pipelines.emplace_back(
        std::in_place_type<RayTracingPipeline>,
        device,
        shaders
    );

    LOG_INFO("Pipeline") << "Created ray tracing pipeline: " << name;
    return std::get<RayTracingPipeline>(variant);
  }

  /**
   * @brief Get a pipeline by name (returns base type pointer)
   * @param name Pipeline identifier
   * @return Pointer to pipeline base, or nullptr if not found
   */
  PipelineBase *getPipeline(const std::string &name);

  /**
   * @brief Get a typed pipeline by name
   * @tparam T Pipeline type (GraphicsPipeline, ComputePipeline, etc.)
   * @param name Pipeline identifier
   * @return Pointer to typed pipeline, or nullptr if not found or wrong type
   */
  template <std::derived_from<PipelineBase> T>
  T *getPipelineAs(const std::string &name)
  {
    auto it = pipelineIndices.find(name);
    if (it == pipelineIndices.end()) {
      return nullptr;
    }

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
  void bindPipeline(const std::string &name, VkCommandBuffer commandBuffer);

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
  [[nodiscard]] bool hasPipeline(const std::string &name) const noexcept
  {
    return pipelineIndices.contains(name);
  }
};

#endif // PIPELINE_MANAGER_HPP
