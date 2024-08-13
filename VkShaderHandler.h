#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <iostream>
#include <map>

#define FILE_DEBUG
// Define DEBUG_PRINT only if DEBUG is enabled
#ifdef FILE_DEBUG
    #define FILE_DEBUG_PRINT(fmt, ...) \
        printf("FILE_DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
    #define FILE_DEBUG_PRINT(fmt, ...) \
        // Nothing, as DEBUG_PRINT is empty in release mode
#endif

/** @brief map matching an extension to a shader bit
 * same shader can be used in different stages the infomation
 * provided is mostly to categorise them then endvalue of the
 * shader flag mask
 */
static const std::map<std::string,VkShaderStageFlagBits> shaderFileExtensions{
  {".vert",  VK_SHADER_STAGE_VERTEX_BIT},
  {".frag",  VK_SHADER_STAGE_FRAGMENT_BIT},
  {".comp",  VK_SHADER_STAGE_COMPUTE_BIT},
  {".geom",  VK_SHADER_STAGE_GEOMETRY_BIT},
  {".tesc",  VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
  {".tese",  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
  {".rgen",  VK_SHADER_STAGE_RAYGEN_BIT_KHR},
  {".rahit", VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
  {".rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
  {".rmiss", VK_SHADER_STAGE_MISS_BIT_KHR},
  {".rint",  VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
  {".rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR},
  {".task",  VK_SHADER_STAGE_TASK_BIT_EXT},
  {".mesh",  VK_SHADER_STAGE_MESH_BIT_EXT}
};

/** @brief enum to represent a result status type for files */
typedef enum VkFileResult{
  VK_FILE_SUCCESS         =  0,
  VK_FILE_EXTENTION_ERROR =  1,
  VK_FILE_NO_MATCH_ERROR  =  2,
  VK_FILE_UNKNOWN_ERROR   =  3,
  VK_FILE_EMPTY_FOLDER    =  4,
  VK_NO_FILE_FOUND        =  5
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

/** 
* @brief Shader Handler is a class that will load all shaders from a given path
* @member sDatas vector of Shaders datas
*/
class VkShaderHandler{
  std::vector<ShaderDataFile> sDatas; 
  
  uint64_t validateExtention(std::string ext){
    std::cout << "Extension : "<< ext<<std::endl;
    return 0;
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
        listFiles(folder); // TODO
        uint64_t a = LoadDataFile(folder); //TODO
      }
    }

    uint32_t listFiles(std::filesystem::path folder){
      std::filesystem::path current_dir = folder;
      for (auto a : std::filesystem::directory_iterator(current_dir))
        if(std::filesystem::is_regular_file(a.status()))
          std::cout<<"current path :" <<a.path().filename().string() << std::endl;
      return sDatas.size();
    };

    [[nodiscard]] VkFileResult LoadDataFile(const std::filesystem::path& filePath){
      if(std::filesystem::exists(filePath)){
        FILE_DEBUG_PRINT("Given path found and loaded : %s \n", filePath.string().c_str());
        std::string extension = filePath.extension().string();
        uint64_t mask = validateExtention(extension);
        if(mask){
          FILE_DEBUG_PRINT("Extension Loaded : %s mask %lu \n",extension.c_str(),mask);

          return VK_FILE_SUCCESS;
        }
        else {
          FILE_DEBUG_PRINT("Extension couldn't be loaded : %s \n",extension.c_str());
          return VK_FILE_NO_MATCH_ERROR;
        }
      }
      else {
        FILE_DEBUG_PRINT("At given path : %s - No shader elements found : ERROR \n", filePath.string().c_str());
        return VK_FILE_UNKNOWN_ERROR;
      }
    }
};
