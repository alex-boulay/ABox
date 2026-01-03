#ifndef GRAPHICS_PIPELINE_HPP
#define GRAPHICS_PIPELINE_HPP

#include "Logger.hpp"
#include "MemoryWrapper.hpp"
#include "PipelineBase.hpp"
#include "SwapchainManager.hpp"
#include <array>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

static const std::array<VkDynamicState, 2> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

// RenderPass is Graphics-specific, not in PipelineBase
DEFINE_VK_MEMORY_WRAPPER(VkRenderPass, RenderPass, vkDestroyRenderPass)

class GraphicsPipeline : public PipelineBase {
  RenderPassWrapper renderPass;

  VkViewport viewport;
  VkRect2D   scissor;

  VkResult CreateRenderPass(const SwapchainManager &sm, VkDevice device);

  /**
   * @brief Validate that required shader stages are present for graphics
   * pipeline
   * @param shaders Range of shader data files
   * @throws std::runtime_error if required stages are missing
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  static void validateGraphicsShaderStages(const R &shaders)
  {
    bool hasVertex   = false;
    bool hasFragment = false;

    for (const auto &shader : shaders) {
      const auto           &reflectModule = shader.getReflectModule();
      VkShaderStageFlagBits stage =
          static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
      if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
        hasVertex = true;
      }
      else if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        hasFragment = true;
      }
    }

    if (!hasVertex || !hasFragment) {
      std::string missing;
      if (!hasVertex && !hasFragment) {
        missing = "vertex and fragment shaders";
      }
      else if (!hasVertex) {
        missing = "vertex shader";
      }
      else {
        missing = "fragment shader";
      }
      throw std::runtime_error(
          "GraphicsPipeline requires both vertex and fragment shaders "
          "(missing: " +
          missing + ")"
      );
    }
  }

  /**
   * @brief Order graphics shaders by pipeline stage
   * @param shaders Input range of shaders
   * @return Ordered vector of shaders (vertex → tess control → tess eval →
   * geometry → fragment)
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  static std::vector<const ShaderDataFile *>
      orderGraphicsShaders(const R &shaders)
  {
    std::vector<const ShaderDataFile *> ordered;

    auto findStage = [&shaders](VkShaderStageFlagBits targetStage
                     ) -> const ShaderDataFile * {
      for (const auto &shader : shaders) {
        if (!shader.isReflectionValid()) {
          continue;
        }
        const auto           &reflectModule = shader.getReflectModule();
        VkShaderStageFlagBits stage =
            static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
        if (stage == targetStage) {
          return &shader;
        }
      }
      return nullptr;
    };

    // Graphics pipeline order
    const VkShaderStageFlagBits stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    for (auto stage : stages) {
      if (const ShaderDataFile *shader = findStage(stage)) {
        ordered.push_back(shader);
      }
    }

    return ordered;
  }

  /**
   * @brief Validate shader interfaces between consecutive graphics pipeline
   * stages
   * @param shaders Range of shader data files
   */
  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, ShaderDataFile> ||
             std::same_as<std::ranges::range_value_t<R>, const ShaderDataFile>
  static void validateGraphicsShaderInterfaces(const R &shaders)
  {
    auto ordered = orderGraphicsShaders(shaders);

    if (ordered.empty()) {
      LOG_WARN("Shader") << "No valid shaders found for interface validation";
      return;
    }

    // Validate vertex shader outputs gl_Position
    if (ordered[0]) {
      const auto           &reflectModule = ordered[0]->getReflectModule();
      VkShaderStageFlagBits stage =
          static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);

      if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
        bool        hasGlPosition  = false;
        const auto &reflectionData = ordered[0]->getReflectionData();

        LOG_DEBUG("Shader")
            << "Vertex shader has " << reflectionData.outputVariableCount
            << " output variables";

        for (uint32_t i = 0; i < reflectionData.outputVariableCount; ++i) {
          const SpvReflectInterfaceVariable *var =
              reflectionData.outputVariables[i];
          LOG_DEBUG("Shader"
          ) << "  Output "
            << i << ": name=" << (var->name ? var->name : "<null>")
            << ", location=" << var->location
            << ", built_in=" << static_cast<int>(var->built_in) << " (hex: 0x"
            << std::hex << static_cast<uint32_t>(var->built_in) << std::dec
            << ")"
            << ", SpvBuiltInPosition=" << static_cast<int>(SpvBuiltInPosition);

          // Check if this is gl_Position (could be identified by built_in OR by
          // having no/empty name with invalid location)
          bool isGlPosition = (var->built_in == SpvBuiltInPosition) ||
                              ((var->name == nullptr || var->name[0] == '\0') &&
                               var->location == static_cast<uint32_t>(-1));

          if (isGlPosition) {
            LOG_DEBUG("Shader") << "  -> Identified as gl_Position";
            hasGlPosition = true;
            break;
          }
        }

        if (!hasGlPosition) {
          LOG_ERROR("Shader")
              << "Vertex shader '" << ordered[0]->getName()
              << "' must output gl_Position (SpvBuiltInPosition)";
        }
      }
    }

    // Validate interfaces between consecutive stages
    for (size_t i = 0; i < ordered.size() - 1; ++i) {
      const ShaderDataFile *currentStage = ordered[i];
      const ShaderDataFile *nextStage    = ordered[i + 1];

      const auto &currentReflection = currentStage->getReflectionData();
      const auto &nextReflection    = nextStage->getReflectionData();

      // Check each output from current stage
      for (uint32_t outIdx = 0; outIdx < currentReflection.outputVariableCount;
           ++outIdx) {
        const SpvReflectInterfaceVariable *outputVar =
            currentReflection.outputVariables[outIdx];

        // Skip built-ins and variables without valid locations
        if (outputVar->built_in != static_cast<SpvBuiltIn>(-1) ||
            outputVar->location == static_cast<uint32_t>(-1)) {
          continue;
        }

        // Find matching input in next stage
        bool found = false;
        for (uint32_t inIdx = 0; inIdx < nextReflection.inputVariableCount;
             ++inIdx) {
          const SpvReflectInterfaceVariable *inputVar =
              nextReflection.inputVariables[inIdx];

          if (inputVar->location == outputVar->location) {
            found = true;

            // Validate format compatibility
            if (inputVar->format != outputVar->format) {
              LOG_ERROR("Shader")
                  << "Shader interface mismatch at location "
                  << outputVar->location << ": '" << currentStage->getName()
                  << "' outputs "
                  << PipelineBase::formatToString(outputVar->format)
                  << " (variable: "
                  << (outputVar->name ? outputVar->name : "<unnamed>") << ")"
                  << " but '" << nextStage->getName() << "' expects "
                  << PipelineBase::formatToString(inputVar->format)
                  << " (variable: "
                  << (inputVar->name ? inputVar->name : "<unnamed>") << ")";
            }
            break;
          }
        }

        if (!found) {
          LOG_ERROR("Shader")
              << "Shader interface mismatch: '" << currentStage->getName()
              << "' outputs location " << outputVar->location << " (variable: "
              << (outputVar->name ? outputVar->name : "<unnamed>") << ")"
              << " but '" << nextStage->getName() << "' doesn't consume it";
        }
      }

      // Check for unmatched inputs in next stage
      for (uint32_t inIdx = 0; inIdx < nextReflection.inputVariableCount;
           ++inIdx) {
        const SpvReflectInterfaceVariable *inputVar =
            nextReflection.inputVariables[inIdx];

        // Skip built-ins and variables without valid locations
        if (inputVar->built_in != static_cast<SpvBuiltIn>(-1) ||
            inputVar->location == static_cast<uint32_t>(-1)) {
          continue;
        }

        bool hasMatchingOutput = false;
        for (uint32_t outIdx = 0;
             outIdx < currentReflection.outputVariableCount;
             ++outIdx) {
          const SpvReflectInterfaceVariable *outputVar =
              currentReflection.outputVariables[outIdx];
          if (outputVar->location == inputVar->location) {
            hasMatchingOutput = true;
            break;
          }
        }

        if (!hasMatchingOutput) {
          LOG_ERROR("Shader")
              << "Shader interface mismatch: '" << nextStage->getName()
              << "' expects input at location " << inputVar->location
              << " (variable: "
              << (inputVar->name ? inputVar->name : "<unnamed>") << ")"
              << " but '" << currentStage->getName() << "' doesn't provide it";
        }
      }
    }
  }

   public:
  DELETE_MOVE(GraphicsPipeline)
  DELETE_COPY(GraphicsPipeline)

  /**
   * @brief Construct a graphics pipeline
   * @param device Logical device handle
   * @param swapchain Swapchain manager for render pass configuration
   * @param shaders Range of shader data files (accepts any container)
   */
  template <std::ranges::input_range R>
    requires std::same_as<
                 std::remove_cv_t<std::ranges::range_value_t<R>>,
                 ShaderDataFile>
  GraphicsPipeline(
      VkDevice                device,
      const SwapchainManager &swapchain,
      const R                &shaders
  )
      : PipelineBase(device, shaders)
      , renderPass(device)
  {
    validateGraphicsShaderStages(shaders);
    validateGraphicsShaderInterfaces(shaders);

    LOG_DEBUG("Pipeline") << "Device value: " << (void *)device;
    VkResult res = CreateRenderPass(swapchain, device);

    // Create shader modules and stages
    std::vector<ShaderModuleWrapper>             shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderModules.reserve(std::ranges::size(shaders));
    shaderStages.reserve(std::ranges::size(shaders));

    for (const auto &shader : shaders) {
      VkShaderModuleCreateInfo moduleInfo = shader;
      shaderModules.emplace_back(device);

      VkResult result = vkCreateShaderModule(
          device,
          &moduleInfo,
          nullptr,
          shaderModules.back().ptr()
      );

      if (result != VK_SUCCESS) {
        throw std::runtime_error(
            "Failed to create shader module for graphics pipeline"
        );
      }

      shaderStages.push_back(shader.getPSSCI(shaderModules.back()));
    }

    updateExtent(swapchain.getExtent());

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data()
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .vertexBindingDescriptionCount   = 0u,
        .pVertexBindingDescriptions      = nullptr,
        .vertexAttributeDescriptionCount = 0u,
        .pVertexAttributeDescriptions    = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0u,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0u,
        .viewportCount = 1u,
        .pViewports    = &viewport,
        .scissorCount  = 1u,
        .pScissors     = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = 0u,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable         = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = 1u,
        .pAttachments    = &colorBlendAttachment,
        .blendConstants  = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    LOG_DEBUG("Pipeline") << "Shader Stages loading into Pipeline Info";
    for (const VkPipelineShaderStageCreateInfo &a : shaderStages) {
      LOG_DEBUG("Pipeline") << "  Stage: " << a.stage << " - entry point: \""
                            << std::string(a.pName) << "\"";
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0u,
        .stageCount          = static_cast<uint32_t>(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = nullptr,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = renderPass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1,
    };

    res = vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        pipeline.ptr()
    );

    if (res != VK_SUCCESS) {
      std::stringstream ss;
      ss << "Failed to create the graphics pipeline !\n\tError value : " << res
         << std::endl;
      throw std::runtime_error(ss.str().c_str());
    }

    LOG_INFO("Pipeline") << "GraphicsPipeline created successfully";
    printReflectionInfo();
  }

  ~GraphicsPipeline() = default;

  [[nodiscard]] VkPipelineBindPoint getBindPoint() const noexcept override
  {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }

  [[nodiscard]] VkRenderPass getRenderPass() const noexcept
  {
    return renderPass;
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

  void updateExtent(VkExtent2D ext)
  {
    LOG_DEBUG("Pipeline") << "Updating extent in GP width: " << ext.width
                          << " - height " << ext.height;
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
