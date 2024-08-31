#include <asm-generic/errno.h>
#include <cstdio>
#include <glslang/MachineIndependent/Versions.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <array>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cassert> 
#include <algorithm>
#include <optional>
#include <iterator>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#include <glslang/Public/ResourceLimits.h>


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

static const std::map<std::string, SourcePlatform> extensionToPlatform = {
    {".glsl", SourcePlatform::GLSL},
    {".hlsl", SourcePlatform::HLSL},
    {".fx",   SourcePlatform::HLSL},
    {".cl",   SourcePlatform::OpenCL},
    {".cu",   SourcePlatform::CUDA},
    {".wgsl", SourcePlatform::WGSL},
    {".rs",   SourcePlatform::Rust},
    {".py",   SourcePlatform::Python}
};

[[nodiscard]] inline static const SourcePlatform getPlatformExt(std::string platExt){
  return extensionToPlatform.contains(platExt) ? extensionToPlatform.at(platExt) : SourcePlatform::Unknown;
}

static const glslang::EShSource getEShSource(SourcePlatform sourcePlatform){
  switch (sourcePlatform){
    case(SourcePlatform::GLSL) : return glslang::EShSourceGlsl;
    case(SourcePlatform::HLSL) : return glslang::EShSourceHlsl;
    default : return glslang::EShSourceNone;
  }
} //target language for SPIRV compilation

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
  const stageExtention* const stage;    //Pipeline stage for the shader
  const SourcePlatform        platform; // in case of recompilation ?
  
  VkShaderModule              module;   //Loaded no const
  VkDevice *                  device;   //dangerzone ! must exist !

  public:

    ShaderDataFile(const std::string&            name_,
                   const std::vector<uint32_t>&  code_,
                   const stageExtention * const& stage_,
                   const SourcePlatform&         platform_):
      name(name_),code(code_),stage(stage_),platform(platform_){}

    /**
     * @brief load a ShaderModule into the target device 
     * @param the targetted device
     * @return true if it allocated one
     */
    bool loadInstance(VkDevice device_){
      if(device)
        removeInstance();

      VkShaderModuleCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .flags = std::get<VkShaderStageFlagBits>(*stage), // maybe more than one in the future ? to overload with Shader creation.
        .codeSize = code.size(),
        .pCode = code.data()
      };

      FILE_DEBUG_PRINT("Creating shader module from file %s to device %p", name.c_str(),device);
      return (vkCreateShaderModule(*device, &createInfo,nullptr,&module) == VK_SUCCESS);
    }

    /** @brief destroy Shader Module if one already exist
     * @return true if it destroyed false otherwise
     * */
    bool removeInstance(){
      if(device){
        vkDestroyShaderModule(*device,module, nullptr);
        device = nullptr;
        return true;
      }
      return static_cast<bool>(device);
    }

    ~ShaderDataFile(){
      removeInstance();
    }
};
//Device mandatory to create Shaders
//TODO Shader Compilations ?  Done, to Test 
//TODO extension heavier check with platform and preload of the future shader elements
//TODO VkShader Loading and binding 

//TODO free structures ? Shader bindings ? 
//TODO inlude ImGui loading interface ? 
//Make a config file ? - Handle external compilation ? 


[[nodiscard]] static const VkFileResult readExtentions(
  std::filesystem::path filename,
  SourcePlatform* platform,
  const stageExtention** targetStage ){
  
  std::vector<std::string> extensions;

  std::cout << "Test debug " << filename.string() <<std::endl;
  while (filename.has_extension() && extensions.size()<2){
    FILE_DEBUG_PRINT("Extension : %s",filename.extension().c_str());
    extensions.push_back(filename.extension().string());
    filename = filename.stem();
  }
  if (!extensions.size() || !StageExtentionHandler::contains(extensions.back())){
    FILE_DEBUG_PRINT("FAILED DUE TO %d or %d %s",!extensions.size(), !StageExtentionHandler::contains(extensions.back()),extensions.back().c_str());
    return VK_FILE_EXTENTION_ERROR;
  }
  *targetStage = &(*StageExtentionHandler::at(extensions.back()));
  extensions.pop_back();
  if(extensions.size() && extensionToPlatform.contains(extensions.back()))
    *platform = extensionToPlatform.at(extensions.back());
  FILE_DEBUG_PRINT("readExtensions Success");
  return VK_FILE_SUCCESS;
}

/** 
* @brief Shader Handler is a class that will load all shaders from a given path
* @member sDatas vector of Shaders datas
*/
class VkShaderHandler{
  bool isGlsInit = false;
  std::vector<ShaderDataFile> sDatas; 

  /**
   * @brief function to initialize gls
   * @return true if GlsInit is already init or init succeded else false
   */
  inline bool initGlsLang(){
    return isGlsInit ? true : isGlsInit = glslang::InitializeProcess();
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
      for (const std::filesystem::path folder : folderNames){
        loadShaderDataFromFiles(folder);
      }
    }

    ~VkShaderHandler(){
      finalizeGlsLang();
    }


    [[nodiscard]] VkFileResult loadShaderDataFile(const std::filesystem::path& filePath){
      if(std::filesystem::exists(filePath)){
        FILE_DEBUG_PRINT("Given path found : %s", filePath.string().c_str());
        SourcePlatform * platform;
        const stageExtention **  targetStage;
        VkFileResult validExt = readExtentions(filePath, platform, targetStage);
        if(validExt == VK_FILE_SUCCESS){
          FILE_DEBUG_PRINT("Extension Loaded");
          std::string shaderF= LoadShaderFromFile(filePath);
          FILE_DEBUG_PRINT("temp debug Shader Loaded");
          std::vector<uint32_t> code = CompileGLSLToSPIRV( shaderF, get<EShLanguage>(**targetStage));
          FILE_DEBUG_PRINT("temp debug Compiled total number of doubleW: %lu",code.size());
          sDatas.push_back( ShaderDataFile(filePath.filename().string(),code,*targetStage,*platform) );
          FILE_DEBUG_PRINT("AFTER !! ");
          return VK_FILE_SUCCESS;
        }
        else {
          FILE_DEBUG_PRINT("Extension couldn't be loaded : %s ",filePath.c_str());
          return VK_FILE_NOT_A_SHADER;
        }
      }
      else {
        FILE_DEBUG_PRINT("At given path : %s - No shader elements found : ERROR ", filePath.string().c_str());
        return VK_FILE_UNKNOWN_ERROR;
      }
    }
    /**
    * @brief load data from given shader folder with expected extensions
    * @param filesystem::path the directory to load from
    * @return uint16_t the number of loaded file into the vector
    */
    uint32_t loadShaderDataFromFiles(const std::filesystem::path& dirPath){
      uint32_t count = 0;
      
      if (!std::filesystem::is_directory(dirPath))
        std::cerr << "The path is not a directory or does not exist." << std::endl;

      for (auto f : std::filesystem::directory_iterator(dirPath))
        count += std::filesystem::is_regular_file(f) && loadShaderDataFile(f) == VK_FILE_SUCCESS;

      return count;
    }
    /**
    * @brief load a shader from a file to a std::string 
    * @param shaderFile the std::filesystem::path to the file 
    * @return std::string the stringified version of the file
    */
    std::string LoadShaderFromFile(const std::filesystem::path shaderFile){
      std::ifstream file(shaderFile);
      if(!file.is_open()) throw std::runtime_error(std::string("Couldn't load Shader File")+ shaderFile.string());
      return std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
    }

  
    /**
    * @brief Compile a GLSL Shader to Spriv
    * @return a vector of compiled binary data representing the Spriv executable
    */
    std::vector<uint32_t> CompileGLSLToSPIRV( const std::string& shaderCode, EShLanguage shaderStage){
      glslang::TShader shader(shaderStage);
      const std::array<const char *,1> shaderStrings = {shaderCode.c_str()};

      shader.setStrings(shaderStrings.data(),shaderStrings.size());
      shader.setEnvInput( glslang::EShSourceGlsl,  shaderStage, glslang::EShClientVulkan, OPENGL_CHOSEN_VERSION);
      shader.setEnvClient(glslang::EShClientVulkan,VULKAN_CHOSEN_VERSION);
      shader.setEnvTarget(glslang::EShTargetSpv,   SPIRV_CHOSEN_VERSION );

      if(!shader.parse(GetDefaultResources(),100,false, EShMsgDefault)){
        FILE_DEBUG_PRINT("GLSL Parsing Failed for shader stage: %d ",shaderStage);
        FILE_DEBUG_PRINT("%s\n%s",shader.getInfoLog(),shader.getInfoDebugLog());
        return {};
      }

      glslang::TProgram program;
      program.addShader(&shader);

      if(!program.link(EShMsgDefault)){
        FILE_DEBUG_PRINT("GLSL Linking Failed : %s",program.getInfoLog());
        FILE_DEBUG_PRINT("%s",program.getInfoDebugLog());
        return {};
      }

      std::vector<uint32_t> spirv;
      glslang::TIntermediate* intermediate = program.getIntermediate(shaderStage);
      if(!intermediate){
        FILE_DEBUG_PRINT("Failed to get intermediate representation");    
      }  
      
      glslang::SpvOptions spvOptions;
      spv::SpvBuildLogger logger;
      glslang::GlslangToSpv(*intermediate, spirv, &logger, &spvOptions);
      
      FILE_DEBUG_PRINT("\nSpirv Logger\n\n%s\n\nEnd Spirv Logs\n\n",logger.getAllMessages().c_str());

      return spirv;
    }
};
