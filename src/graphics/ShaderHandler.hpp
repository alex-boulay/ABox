#ifndef SHADER_HANDLER_HPP
#define SHADER_HANDLER_HPP

#include "MemoryWrapper.hpp"
#include "PreProcUtils.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <glslang/MachineIndependent/Versions.h>
#include <glslang/Public/ShaderLang.h>
#include <iostream>
#include <list>
#include <optional>
#include <spirv_reflect.h>
#include <string>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>

#define OPENGL_CHOSEN_VERSION 450
#define VULKAN_CHOSEN_VERSION                                                  \
  glslang::EShTargetVulkan_1_3 // need to bind it to current API number
#define SPIRV_CHOSEN_VERSION glslang::EShTargetSpv_1_5

const char MAIN_ENTRY_POINT[5] = "main";

typedef std::tuple<std::string, EShLanguage, VkShaderStageFlagBits>
    stageExtention;

class StageExtentionHandler {
  StageExtentionHandler() = default;
  /** @brief map matching an extension to a shader bit
   * same shader can be used in different stages the infomation
   * provided is mostly to categorise them then endvalue of the
   * shader flag mask
   */
  inline static const std::array<stageExtention, 14> map{
      {{".vert", EShLangVertex, VK_SHADER_STAGE_VERTEX_BIT},
       {".frag", EShLangFragment, VK_SHADER_STAGE_FRAGMENT_BIT},
       {".comp", EShLangCompute, VK_SHADER_STAGE_COMPUTE_BIT},
       {".geom", EShLangGeometry, VK_SHADER_STAGE_GEOMETRY_BIT},
       {".tesc", EShLangTessControl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
       {".tese",
        EShLangTessEvaluation,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
       {".rgen", EShLangRayGen, VK_SHADER_STAGE_RAYGEN_BIT_KHR},
       {".rahit", EShLangAnyHit, VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
       {".rchit", EShLangClosestHit, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
       {".rmiss", EShLangMiss, VK_SHADER_STAGE_MISS_BIT_KHR},
       {".rint", EShLangIntersect, VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
       {".rcall", EShLangCallable, VK_SHADER_STAGE_CALLABLE_BIT_KHR},
       {".task", EShLangTask, VK_SHADER_STAGE_TASK_BIT_EXT},
       {".mesh", EShLangMesh, VK_SHADER_STAGE_MESH_BIT_EXT}}
  };

   public:
  template <typename T>
  inline static constexpr auto at(
      T key
  )
  {
    return std::find_if(map.begin(), map.end(), [key](const auto &v) {
      return key == std::get<T>(v);
    });
  }

  template <typename T>
  inline static constexpr bool contains(
      T key
  )
  {
    return map.end() != at(key);
  }

  [[nodiscard]] inline std::optional<EShLanguage> getStageExt(
      std::string stageExt
  ) noexcept
  {
    return StageExtentionHandler::contains(stageExt)
               ? std::optional<EShLanguage>{std::get<EShLanguage>(
                     *(StageExtentionHandler::at(stageExt))
                 )}
               : std::nullopt;
  }
};

enum class SourcePlatform {
  GLSL,   // GLSL (OpenGL Shading Language)
  HLSL,   // HLSL (High-Level Shading Language, DirectX)
  OpenCL, // OpenCL C
  CUDA,   // CUDA (NVIDIA)
  WGSL,   // WGSL (WebGPU Shading Language)
  Rust,   // Rust (using Vulkan bindings)
  Python, // Python (using PyOpenCL or PyVulkan)
  Unknown // platform not specified or unknown
};

// extern const std::map<std::string, SourcePlatform> extensionToPlatform;

[[nodiscard]] inline SourcePlatform getPlatformExt(std::string platExt);

[[nodiscard]] inline glslang::EShSource
    getEShSource(SourcePlatform sourcePlatform);

/** @brief enum to represent a result status type for files */
typedef enum VkFileResult {
  VK_FILE_SUCCESS         = 0,
  VK_FILE_EXTENTION_ERROR = 1,
  VK_FILE_NO_MATCH_ERROR  = 2,
  VK_FILE_UNKNOWN_ERROR   = 3,
  VK_FILE_EMPTY_FOLDER    = 4,
  VK_NO_FILE_FOUND        = 5,
  VK_FILE_NOT_A_SHADER    = 6
} VkFileResult;

DEFINE_VK_MEMORY_WRAPPER(
    VkShaderModule,
    ShaderModule,
    vkDestroyShaderModule
)

/**
 * @brief Structure to hold SPIRV reflection data
 */
struct ShaderReflectionData {
  std::vector<SpvReflectDescriptorSet *>     descriptorSets;
  std::vector<SpvReflectBlockVariable *>     pushConstants;
  std::vector<SpvReflectInterfaceVariable *> inputVariables;
  std::vector<SpvReflectInterfaceVariable *> outputVariables;

  uint32_t descriptorSetCount  = 0;
  uint32_t pushConstantCount   = 0;
  uint32_t inputVariableCount  = 0;
  uint32_t outputVariableCount = 0;

  ~ShaderReflectionData() = default;
};

/**
 * @brief class used to represent a Shader data file
 * @member data contains the file data as a vector of chars
 * @member mask
 */
class ShaderDataFile {
  const std::string           name;  // name without extensions
  const std::vector<uint32_t> code;  // Spirv output
  const stageExtention *const stage; // Pipeline stage for the shader
  [[maybe_unused]] const SourcePlatform
                         platform; // in case of recompilation ?
                                   // ShaderDataFile doesn't handle allocations
  SpvReflectShaderModule reflectModule;
  ShaderReflectionData   reflectionData;
  bool                   reflectionValid = false;

   public:
  ShaderDataFile(
      const std::string           &name_,
      const std::vector<uint32_t> &code_,
      const stageExtention *const  stage_,
      const SourcePlatform        &platform_
  )
      : name(name_)
      , code(code_)
      , stage(stage_)
      , platform(platform_)
  {
    performReflection();
  }

  ~ShaderDataFile()
  {
    if (reflectionValid) {
      spvReflectDestroyShaderModule(&reflectModule);
    }
  }

  inline operator VkShaderModuleCreateInfo() const
  {
    return {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0u, // maybe more than one in the future ? to overload
                        // with Shader creation.
        .codeSize = code.size() * sizeof(uint32_t),
        .pCode    = code.data()
    };
  }

  DELETE_COPY(ShaderDataFile);
  DELETE_MOVE(ShaderDataFile);

  std::string getName() const { return name; }

  [[nodiscard]] inline VkPipelineShaderStageCreateInfo getPSSCI(
      VkShaderModule shm
  ) const
  {
    return {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext  = nullptr,
        .flags  = 0u,
        .stage  = std::get<VkShaderStageFlagBits>(*stage),
        .module = shm,
        .pName  = MAIN_ENTRY_POINT,
        .pSpecializationInfo = nullptr
    };
  }

  const ShaderReflectionData &getReflectionData() const
  {
    return reflectionData;
  }
  bool isReflectionValid() const { return reflectionValid; }
  const SpvReflectShaderModule &getReflectModule() const
  {
    return reflectModule;
  }

   private:
  void performReflection();
};

/**
 * @brief Shader Handler is a class that will load all shaders from a given
 * path
 * @member sDatas vector of Shaders datas
 */
class ShaderHandler {
  bool                      isGlsInit = false;
  std::list<ShaderDataFile> sDatas;

  /**
   * @brief function to initialize gls
   * @return true if GlsInit is already init or init succeded else false
   */
  inline bool initGlsLang()
  {
    return isGlsInit ? true : (isGlsInit = glslang::InitializeProcess());
  }

  /**@brief uninitialise glsl*/
  inline void finalizeGlsLang()
  {
    if (isGlsInit) {
      glslang::FinalizeProcess();
      isGlsInit = false;
    }
  }

   public:
  ShaderHandler()
      : ShaderHandler(SHADER_DIR)
  {
  }
  ShaderHandler(
      std::filesystem::path folder
  )
      : ShaderHandler({folder})
  {
  }

  ShaderHandler(
      std::initializer_list<std::filesystem::path> folderNames
  )
  {
    initGlsLang();
    for (const std::filesystem::path &folder : folderNames) {
      loadShaderDataFromFolder(folder);
    }
    std::cout << "-- " << sDatas.size() << " Shaders Loaded !" << std::endl;
  }

  ~ShaderHandler()
  {
    std::cout << "Destruction of the Shader Handler" << std::endl;
    finalizeGlsLang();
  }

  ShaderHandler(const ShaderHandler &other
  )          = delete; // no Copy -> ShaderDataFile are unique
  ShaderHandler &operator=(const ShaderHandler &other
  ) noexcept = delete; // no Copy
  ShaderHandler(ShaderHandler &&other)                     = default;
  ShaderHandler &operator=(ShaderHandler &&other) noexcept = default;
  /**
   * @brief load data from given shader folder with expected extensions
   * @param filesystem::path the directory to load from
   * @return uint16_t the number of loaded files into the vector
   */
  uint32_t       loadShaderDataFromFolder(const std::filesystem::path &dirPath);

  /**
   * @brief
   * @return std::string
   */
  const std::string loadShaderFromFile(const std::filesystem::path &shaderFile);

  /**
   * @brief load a shader from a file to a std::string
   * @param shaderFile the std::filesystem::path to the file
   * @return std::string the stringified version of the file
   */
  [[nodiscard]] VkFileResult
      loadShaderDataFile(const std::filesystem::path &filePath);

  /**
   * @brief Compile a GLSL Shader to Spriv
   * @return a vector of compiled binary data representing the Spriv
   * executable
   */
  const std::vector<uint32_t> compileGLSLToSPIRV(
      const std::string &shaderCode,
      const EShLanguage &shaderStage
  );

  std::string listAllShaders() const;

  const ShaderDataFile *getShader(std::string name) const;

  [[nodiscard]] explicit operator std::vector<VkShaderModuleCreateInfo>() const;

  inline const std::list<ShaderDataFile> &getShaderHandlers() const
  {
    std::cout << " Loading sDatas - Size :" << sDatas.size() << std::endl;
    return sDatas;
  }
};

#endif // SHADER_HANDLER_HPP
