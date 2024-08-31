#include "VkShaderHandler.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <map>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#include <glslang/Public/ResourceLimits.h>

// ---------------Static Types---------------------

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

//----------------Static Functions-----------------
[[nodiscard]] inline static const SourcePlatform getPlatformExt(std::string platExt){
  return extensionToPlatform.contains(platExt) ? extensionToPlatform.at(platExt) : SourcePlatform::Unknown;
}

[[nodiscard]] static const glslang::EShSource getEShSource(SourcePlatform sourcePlatform){
  switch (sourcePlatform){
    case(SourcePlatform::GLSL) : return glslang::EShSourceGlsl;
    case(SourcePlatform::HLSL) : return glslang::EShSourceHlsl;
    default : return glslang::EShSourceNone;
  }
}

[[nodiscard]] static const VkFileResult readExtentions(
  std::filesystem::path filename,        //IN
  SourcePlatform& platform,              // OUT
  const stageExtention ** targetStage ){ //OUT
  
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
    platform = extensionToPlatform.at(extensions.back());
  FILE_DEBUG_PRINT("readExtensions Success");
  return VK_FILE_SUCCESS;
}

//----------------ShaderDataFile::Functions---------------

bool ShaderDataFile::loadInstance(VkDevice device_){
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

bool ShaderDataFile::removeInstance(){
  if(device){
    vkDestroyShaderModule(*device,module, nullptr);
    device = nullptr;
    return true;
  }
  return static_cast<bool>(device);
}

//--------------- VkShaderHandler -----------------

[[nodiscard]] VkFileResult VkShaderHandler::loadShaderDataFile(const std::filesystem::path& filePath){
  if(std::filesystem::exists(filePath)){
    FILE_DEBUG_PRINT("Given path found : %s", filePath.string().c_str());
    SourcePlatform platform = SourcePlatform::Unknown;
    const stageExtention* targetStage;
    VkFileResult validExt = readExtentions(filePath, platform, &targetStage);
    if(validExt == VK_FILE_SUCCESS){
      FILE_DEBUG_PRINT("Extension(s) Loaded");
      std::string shaderF= loadShaderFromFile(filePath);
      FILE_DEBUG_PRINT("Shader Loaded");
      std::vector<uint32_t> code = compileGLSLToSPIRV( shaderF, get<EShLanguage>(*targetStage));
      FILE_DEBUG_PRINT("Compiled total number of doubleW: %lu",code.size());
      sDatas.push_back( ShaderDataFile(filePath.filename().string(),code,targetStage,platform));
      FILE_DEBUG_PRINT("ShaderData added");
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

uint32_t VkShaderHandler::loadShaderDataFromFolder(const std::filesystem::path& dirPath){
  uint32_t count = 0;
  
  if (!std::filesystem::is_directory(dirPath))
    std::cerr << "The path is not a directory or does not exist." << std::endl;

  for (auto f : std::filesystem::directory_iterator(dirPath))
    count += std::filesystem::is_regular_file(f) && loadShaderDataFile(f) == VK_FILE_SUCCESS;

  return count;
}

std::string VkShaderHandler::loadShaderFromFile(const std::filesystem::path shaderFile){
  std::ifstream file(shaderFile);
  if(!file.is_open()) throw std::runtime_error(std::string("Couldn't load Shader File")+ shaderFile.string());
  return std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
}


std::vector<uint32_t> VkShaderHandler::compileGLSLToSPIRV( const std::string& shaderCode, EShLanguage shaderStage){
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
  
  if(logger.getAllMessages().size())
    std::cout << "Spirv Logger : %s" << logger.getAllMessages()<<std::endl;

  return spirv;
}
