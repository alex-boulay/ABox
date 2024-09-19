#include <cstdio>
#include <glslang/MachineIndependent/Versions.h>
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <cassert> 
#include <algorithm>
#include <optional>
#include <iostream>
#include <glslang/Public/ShaderLang.h>

#pragma once

#define FILE_DEBUG
// Define DEBUG_PRINT only if DEBUG is enabled
#ifdef FILE_DEBUG
    #define FILE_DEBUG_PRINT(fmt, ...) \
        printf("FILE_DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
    #define FILE_DEBUG_PRINT(fmt, ...) \
        // Nothing, as DEBUG_PRINT is empty in release mode
#endif

#define OPENGL_CHOSEN_VERSION 450
#define VULKAN_CHOSEN_VERSION glslang::EShTargetVulkan_1_3 // need to bind it to current API number
#define SPIRV_CHOSEN_VERSION  glslang::EShTargetSpv_1_5

typedef std::tuple<std::string,EShLanguage,VkShaderStageFlagBits> stageExtention;

class StageExtentionHandler{
  
  StageExtentionHandler() = default;
  /** @brief map matching an extension to a shader bit
   * same shader can be used in different stages the infomation
   * provided is mostly to categorise them then endvalue of the
   * shader flag mask
   */
  inline static const std::array<stageExtention,14> map{{
    {".vert" , EShLangVertex        , VK_SHADER_STAGE_VERTEX_BIT},
    {".frag" , EShLangFragment      , VK_SHADER_STAGE_FRAGMENT_BIT},
    {".comp" , EShLangCompute       , VK_SHADER_STAGE_COMPUTE_BIT},
    {".geom" , EShLangGeometry      , VK_SHADER_STAGE_GEOMETRY_BIT},
    {".tesc" , EShLangTessControl   , VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {".tese" , EShLangTessEvaluation, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {".rgen" , EShLangRayGen        , VK_SHADER_STAGE_RAYGEN_BIT_KHR},
    {".rahit", EShLangAnyHit        , VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
    {".rchit", EShLangClosestHit    , VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
    {".rmiss", EShLangMiss          , VK_SHADER_STAGE_MISS_BIT_KHR},
    {".rint" , EShLangIntersect     , VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
    {".rcall", EShLangCallable      , VK_SHADER_STAGE_CALLABLE_BIT_KHR},
    {".task" , EShLangTask          , VK_SHADER_STAGE_TASK_BIT_EXT},
    {".mesh" , EShLangMesh          , VK_SHADER_STAGE_MESH_BIT_EXT}
  }};

public:

  template<typename T>
  inline static constexpr auto at(T key){ return std::find_if(map.begin(),map.end(),[key](const auto& v){return key == std::get<T>(v);});}
  
  template<typename T>
  inline static constexpr bool contains(T key) { return map.end()!= at(key);}

  [[nodiscard]] inline static constexpr std::optional<EShLanguage> getStageExt(std::string stageExt){
    return StageExtentionHandler::contains(stageExt) ? 
      std::optional<EShLanguage>{std::get<EShLanguage>(*(StageExtentionHandler::at(stageExt)))} :
      std::nullopt;
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

//extern const std::map<std::string, SourcePlatform> extensionToPlatform;

//[[nodiscard]] inline extern constexpr SourcePlatform getPlatformExt(std::string platExt);

//[[nodiscard]] extern const glslang::EShSource getEShSource(SourcePlatform sourcePlatform);

/** @brief enum to represent a result status type for files */
typedef enum VkFileResult{
  VK_FILE_SUCCESS         =  0,
  VK_FILE_EXTENTION_ERROR =  1,
  VK_FILE_NO_MATCH_ERROR  =  2,
  VK_FILE_UNKNOWN_ERROR   =  3,
  VK_FILE_EMPTY_FOLDER    =  4,
  VK_NO_FILE_FOUND        =  5,
  VK_FILE_NOT_A_SHADER    =  6
} VkFileResult;

/**
 * @brief class used to represent a Shader data file
 * @member data contains the file data as a vector of chars
 * @member mask
 */
class ShaderDataFile{
  const std::string           name;     //name without extensions
  const std::vector<uint32_t> code;     //Spirv output
  const stageExtention *const stage;    //Pipeline stage for the shader
  const SourcePlatform        platform; // in case of recompilation ?
  
  //Maybe not the best implementation ? modules and devices shall be set elsewhere ? 
  struct shaderBind{
    const VkDevice * device;
    VkShaderModule   module;
  };
  std::vector<shaderBind> sBinds; //map a Shader to a Device

  public:

    ShaderDataFile(const std::string&            name_,
                   const std::vector<uint32_t>&  code_,
                   const stageExtention *const   stage_,
                   const SourcePlatform&         platform_):
      name(name_),code(code_),stage(stage_),platform(platform_){
  }

    /**
     * @brief load a ShaderModule into the target device 
     * @param the targetted device
     * @return true if it allocated one
     */
    [[nodiscard]] VkResult load(const VkDevice * &device_);

    /** @brief unload Shader Module it has an instance 
     * loaded on the given device and free module memory
     * @return true if it destroyed false otherwise
     * */
    [[nodiscard]] VkResult unload(const VkDevice * &device);

    ~ShaderDataFile(){
      for (auto bind : sBinds) (void)unload(bind.device);
    }

  /**  ShaderDataFile(const ShaderDataFile& other);
    ShaderDataFile& operator=(const ShaderDataFile& other) noexcept;
    ShaderDataFile(ShaderDataFile&& other);
    ShaderDataFile& operator=(ShaderDataFile && other) noexcept;
*/
    std::string getName(){ return name;}
};


/** 
* @brief Shader Handler is a class that will load all shaders from a given path
* @member sDatas vector of Shaders datas
*/
class VkShaderHandler{
  bool isGlsInit = false;
  std::vector<ShaderDataFile> sDatas; 
  // Devices are used to bind shader to a device when loading so unloading can be done automaticly;
  std::vector<VkDevice *>     devices; //Usualy one, only exist if shaders are loaded

  /**
   * @brief function to initialize gls
   * @return true if GlsInit is already init or init succeded else false
   */
  inline bool initGlsLang(){
    return isGlsInit ? true : (isGlsInit = glslang::InitializeProcess());
  }

  /**@brief uninitialise glsl*/
  inline void finalizeGlsLang(){
    if (isGlsInit && !(isGlsInit ^= isGlsInit)) glslang::FinalizeProcess();
  }

  public :
    
    VkShaderHandler(){
      VkShaderHandler("../shaders/");
    }
    
    VkShaderHandler(std::filesystem::path folder){
      VkShaderHandler({folder});
    }

    VkShaderHandler(std::initializer_list<std::filesystem::path> folderNames){
      initGlsLang();
      for (const std::filesystem::path& folder : folderNames){
        loadShaderDataFromFolder(folder);
      }
    }

    ~VkShaderHandler(){
      finalizeGlsLang();
    }

    /**VkShaderHandler( const VkShaderHandler& other);
    VkShaderHandler& operator=( const VkShaderHandler& other) noexcept;
    VkShaderHandler( VkShaderHandler&& other);
    VkShaderHandler& operator=( VkShaderHandler&& other) noexcept;
*/ 
    /**
    * @brief load data from given shader folder with expected extensions
    * @param filesystem::path the directory to load from
    * @return uint16_t the number of loaded files into the vector
    */
    uint32_t loadShaderDataFromFolder(const std::filesystem::path& dirPath);
    
    /**
     * @brief 
     * @return std::string
     */
    const std::string loadShaderFromFile(const std::filesystem::path& shaderFile);

    /**
    * @brief load a shader from a file to a std::string 
    * @param shaderFile the std::filesystem::path to the file 
    * @return std::string the stringified version of the file
    */
   [[nodiscard]] VkFileResult loadShaderDataFile(const std::filesystem::path& filePath);
    
    /**
    * @brief Compile a GLSL Shader to Spriv
    * @return a vector of compiled binary data representing the Spriv executable
    */
    const std::vector<uint32_t> compileGLSLToSPIRV( const std::string& shaderCode,const EShLanguage& shaderStage);

    std::string ListAllShaders();

    ShaderDataFile * getShader(std::string name);
    VkResult loadAllShaders(const VkDevice *&device);
};

