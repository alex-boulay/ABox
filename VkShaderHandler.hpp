#include <cstdio>
#include <glslang/MachineIndependent/Versions.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cassert> 
#include <algorithm>
#include <glslang/Public/ShaderLang.h>

#define FILE_DEBUG
// Define DEBUG_PRINT only if DEBUG is enabled
#ifdef FILE_DEBUG
    #define FILE_DEBUG_PRINT(fmt, ...) \
        printf("FILE_DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
    #define FILE_DEBUG_PRINT(fmt, ...) \
        // Nothing, as DEBUG_PRINT is empty in release mode
#endif

class ShaderExtentionHandler{
  
  ShaderExtentionHandler() = default;
  /** @brief map matching an extension to a shader bit
   * same shader can be used in different stages the infomation
   * provided is mostly to categorise them then endvalue of the
   * shader flag mask
   */
  inline static const std::vector<std::tuple<std::string,EShLanguage,VkShaderStageFlagBits>> map={
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
  };

public:

  template<typename T>
  static constexpr auto at(T key){ return std::find_if(map.begin(),map.end(),[key](const auto& v){return key == std::get<T>(v);});}
  
  template<typename T>
  static constexpr bool contains(T key) { return map.end()!= at(key);}

};

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
  const std::string name;
  const std::vector<char> code;
  const int64_t mask;
  VkShaderModule module;
  const VkDevice * const device; //dangerzone !

  public:
    ShaderDataFile(std::string name_, std::vector<char> code_, int64_t mask_,
                   const VkDevice * const device_): name(name_),code(code_),mask(mask_),device(device_){
      VkShaderModuleCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
      };

      if (vkCreateShaderModule(*device, &createInfo,nullptr,&module) != VK_SUCCESS)
          throw std::runtime_error("failed to create shader module");
    }
    ~ShaderDataFile(){
      vkDestroyShaderModule(*device,module, nullptr);
    }
};

//TODO Shader Compilations ? 
//TODO inlude ImGui loading interface ? 
//Make a config file ?

/** 
* @brief Shader Handler is a class that will load all shaders from a given path
* @member sDatas vector of Shaders datas
*/
class VkShaderHandler{
  std::vector<ShaderDataFile> sDatas; 
  
  bool validateExtention(std::string ext){
    FILE_DEBUG_PRINT("Extension : %s",ext.c_str());
    return static_cast<uint64_t>(ShaderExtentionHandler::contains(ext));
  }

  public :
    
    VkShaderHandler(){
      VkShaderHandler("../shaders/");
    }
    
    VkShaderHandler(std::filesystem::path folder){
      VkShaderHandler({folder});
    }

    VkShaderHandler(std::initializer_list<std::filesystem::path> folderNames){
      for (const std::filesystem::path folder : folderNames){
        loadShaderDataFromFiles(folder);
      }
    }

    [[nodiscard]] VkFileResult loadShaderDataFile(const std::filesystem::path& filePath){
      if(std::filesystem::exists(filePath)){
        FILE_DEBUG_PRINT("Given path found : %s", filePath.string().c_str());
        std::string extension = filePath.extension().string();
        bool validExt = validateExtention(extension);
        if(validExt){
          FILE_DEBUG_PRINT("Extension Loaded : %s",extension.c_str());
        std::cout << LoadShaderFromFile(filePath);
          

          return VK_FILE_SUCCESS;
        }
        else {
          FILE_DEBUG_PRINT("Extension couldn't be loaded : %s ",extension.c_str());
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
    std::string LoadShaderFromFile(const std::filesystem::path shaderFile){
      std::ifstream file(shaderFile);
      if(!file.is_open()) throw std::runtime_error(std::string("Couldn't load Shader File")+ shaderFile.string());
      return std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
    }
};
