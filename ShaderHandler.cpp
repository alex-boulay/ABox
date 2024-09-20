#include "ShaderHandler.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <map>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#include <glslang/Public/ResourceLimits.h>
#include <vulkan/vulkan_core.h>

// @brief anonymous namespace to declare static elements
namespace {

  // ---------------Static Types---------------------
  const std::map<std::string, SourcePlatform> extensionToPlatform = {
      {".glsl", SourcePlatform::GLSL},
      {".hlsl", SourcePlatform::HLSL},
      {".fx",   SourcePlatform::HLSL},
      {".cl",   SourcePlatform::OpenCL},
      {".cu",   SourcePlatform::CUDA},
      {".wgsl", SourcePlatform::WGSL},
      {".rs",   SourcePlatform::Rust},
      {".py",   SourcePlatform::Python}
  };

  struct ExtensionFileResult{
          VkFileResult    status;
          SourcePlatform  platform;
  const stageExtention* stage;
  };

  //----------------Static Functions-----------------
  [[nodiscard]] inline SourcePlatform getPlatformExt(std::string platExt){
    return extensionToPlatform.contains(platExt) ? extensionToPlatform.at(platExt) : SourcePlatform::Unknown;
  }

  [[nodiscard]] inline glslang::EShSource getEShSource(SourcePlatform sourcePlatform){
    switch (sourcePlatform){
      case(SourcePlatform::GLSL) : return glslang::EShSourceGlsl;
      case(SourcePlatform::HLSL) : return glslang::EShSourceHlsl;
      default : return glslang::EShSourceNone;
    }
  }

  [[nodiscard]] ExtensionFileResult readExtentions(std::filesystem::path path){
    std::vector<std::string> extensions;
    auto filename = path.filename();
    ExtensionFileResult result{
      .status   = VK_FILE_EXTENTION_ERROR,
      .platform = SourcePlatform::Unknown,
      .stage    = nullptr
    };

    FILE_DEBUG_PRINT(" readExtensions - Filename : %s", filename.c_str());
    while (filename.has_extension() && extensions.size()<2){
      FILE_DEBUG_PRINT("Extension : %s",filename.extension().c_str());
      extensions.push_back(filename.extension().string());
      filename = filename.stem();
    }
    if (!extensions.size() || !StageExtentionHandler::contains(extensions.back())){
      FILE_DEBUG_PRINT("File ignored due to unsuported extension : %s", extensions.back().c_str());
      return result; //.status = VK_FILE_EXTENSION ERROR
    }
    else{
    result.stage = &(*StageExtentionHandler::at(extensions.back()));
    extensions.pop_back();
    if(extensions.size() && extensionToPlatform.contains(extensions.back()))
      result.platform = extensionToPlatform.at(extensions.back());
    FILE_DEBUG_PRINT("readExtensions Success");
    result.status = VK_FILE_SUCCESS;
    }
    return result;
  }
};
//----------------ShaderDataFile::Functions---------------

VkResult ShaderDataFile::load(const VkDevice * &device){
  if(sBinds.end() != std::find_if(sBinds.begin(),sBinds.end(),[device](shaderBind a){ return a.device == device;})){
    return VK_INCOMPLETE;
  }

  VkShaderModuleCreateInfo createInfo {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = nullptr,
    .flags = std::get<VkShaderStageFlagBits>(*stage), // maybe more than one in the future ? to overload with Shader creation.
    .codeSize = code.size(),
    .pCode = code.data()
  };

  shaderBind payload = {
    .device = device,
    .module = {},
  };

  FILE_DEBUG_PRINT("Creating shader module from file %s to device %p", name.c_str(),device);
  VkResult result = vkCreateShaderModule(*device, &createInfo,nullptr, &payload.module);
  FILE_DEBUG_PRINT("Loading Success if 0 == %d !", result);

  if (result == VK_SUCCESS){ 
    sBinds.push_back(payload);
  }
  return result;
}

VkResult ShaderDataFile::unload(const VkDevice * &device){
  std::vector<shaderBind>::iterator target = std::find_if(sBinds.begin(),sBinds.end(),[device](shaderBind a){ return a.device == device;});
  if(target != sBinds.end()){
    shaderBind sb = *target;
    vkDestroyShaderModule(*(sb.device),sb.module,nullptr);
    sBinds.erase(target);
    return VK_SUCCESS;
  }
  return VK_INCOMPLETE;
}

//--------------- ShaderHandler -----------------

[[nodiscard]] VkFileResult ShaderHandler::loadShaderDataFile(const std::filesystem::path& filePath){
  if(std::filesystem::exists(filePath)){
    FILE_DEBUG_PRINT("Given path found : %s", filePath.string().c_str());
    ExtensionFileResult extFileResult = readExtentions(filePath);
    if(extFileResult.status == VK_FILE_SUCCESS){
      FILE_DEBUG_PRINT("Extension(s) Loaded");
      std::string shaderF= loadShaderFromFile(filePath);
      FILE_DEBUG_PRINT("Shader Loaded");
      std::vector<uint32_t> code = compileGLSLToSPIRV( shaderF, get<EShLanguage>(*extFileResult.stage));
      FILE_DEBUG_PRINT("Compiled total number of uint32_t : %lu",code.size());
      sDatas.push_back(ShaderDataFile(filePath.filename(),code,extFileResult.stage,extFileResult.platform));
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

uint32_t ShaderHandler::loadShaderDataFromFolder(const std::filesystem::path& dirPath){
  uint32_t count = 0;
  
  if (!std::filesystem::is_directory(dirPath))
    FILE_DEBUG_PRINT("The path is not a directory or does not exist.");

  for (auto f : std::filesystem::directory_iterator(dirPath)){
    std::cout << " count : "<<count <<" filename:  "<< f.path().filename().string() << '\n';
    count += std::filesystem::is_regular_file(f) && loadShaderDataFile(f) == VK_FILE_SUCCESS;
  }
  return const_cast<uint32_t&>(count);
}

const std::string ShaderHandler::loadShaderFromFile(const std::filesystem::path& shaderFile){
  std::ifstream file(shaderFile);
  if(!file.is_open()) throw std::runtime_error(std::string("Couldn't load Shader File")+ shaderFile.string());
  return std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
}


const std::vector<uint32_t> ShaderHandler::compileGLSLToSPIRV( const std::string& shaderCode,const EShLanguage& shaderStage){
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
    FILE_DEBUG_PRINT("Spirv Logger : %s",logger.getAllMessages().c_str());

  return spirv;
}

std::string ShaderHandler::listAllShaders(){
  return std::accumulate(sDatas.begin(),sDatas.end(),std::string(),[](const std::string& a,const ShaderDataFile &b){return a + b.getName() + '\n';});
}

ShaderDataFile * ShaderHandler::getShader(std::string name){
  std::vector<ShaderDataFile>::iterator result = std::find_if(sDatas.begin(),sDatas.end(),[&name](const ShaderDataFile &a)->bool{ return a.getName()==name;});
  return result != sDatas.end() ? &(*result) : nullptr ;
}

uint32_t ShaderHandler::loadAllShaders(const VkDevice *&device){
  return std::accumulate(sDatas.begin(), sDatas.end(), 0u,
    [&device](uint32_t acc,ShaderDataFile& s)->uint32_t{ return acc + (s.load(device) == VK_SUCCESS);});
}
