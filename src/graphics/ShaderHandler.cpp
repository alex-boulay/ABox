#include "ShaderHandler.hpp"
#include <algorithm>
#include <fstream>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <spirv_reflect.h>

// @brief anonymous namespace to declare static elements
namespace {

// ---------------Static Types---------------------
const std::map<std::string, SourcePlatform> extensionToPlatform = {
    {".glsl",   SourcePlatform::GLSL},
    {".hlsl",   SourcePlatform::HLSL},
    {  ".fx",   SourcePlatform::HLSL},
    {  ".cl", SourcePlatform::OpenCL},
    {  ".cu",   SourcePlatform::CUDA},
    {".wgsl",   SourcePlatform::WGSL},
    {  ".rs",   SourcePlatform::Rust},
    {  ".py", SourcePlatform::Python}
};

struct ExtensionFileResult {
  VkFileResult          status;
  SourcePlatform        platform;
  const stageExtention *stage;
};

//----------------Static Functions-----------------
[[nodiscard]] inline SourcePlatform getPlatformExt(
    std::string platExt
)
{
  return extensionToPlatform.contains(platExt) ? extensionToPlatform.at(platExt)
                                               : SourcePlatform::Unknown;
}

[[nodiscard]] inline glslang::EShSource getEShSource(
    SourcePlatform sourcePlatform
)
{
  switch (sourcePlatform) {
    case (SourcePlatform::GLSL): return glslang::EShSourceGlsl;
    case (SourcePlatform::HLSL): return glslang::EShSourceHlsl;
    default: return glslang::EShSourceNone;
  }
}

[[nodiscard]] ExtensionFileResult readExtentions(
    std::filesystem::path path
)
{
  std::vector<std::string> extensions;
  auto                     filename = path.filename();
  ExtensionFileResult      result{
           .status   = VK_FILE_EXTENTION_ERROR,
           .platform = SourcePlatform::Unknown,
           .stage    = nullptr
  };

  FILE_DEBUG_PRINT(" readExtensions - Filename : %s", filename.c_str());
  while (filename.has_extension() && extensions.size() < 2) {
    FILE_DEBUG_PRINT("Extension : %s", filename.extension().c_str());
    extensions.push_back(filename.extension().string());
    filename = filename.stem();
  }
  if (!extensions.size() ||
      !StageExtentionHandler::contains(extensions.back())) {
    FILE_DEBUG_PRINT(
        "File ignored due to unsuported extension : %s",
        extensions.back().c_str()
    );
    return result; //.status = VK_FILE_EXTENSION ERROR
  }
  else {
    result.stage = &(*StageExtentionHandler::at(extensions.back()));
    extensions.pop_back();
    if (extensions.size() && extensionToPlatform.contains(extensions.back())) {
      result.platform = extensionToPlatform.at(extensions.back());
    }
    FILE_DEBUG_PRINT("readExtensions Success");
    result.status = VK_FILE_SUCCESS;
  }
  return result;
}
}; // namespace
//--------------- ShaderHandler -----------------

[[nodiscard]] VkFileResult ShaderHandler::loadShaderDataFile(
    const std::filesystem::path &filePath
)
{
  if (std::filesystem::exists(filePath)) {
    FILE_DEBUG_PRINT("Given path found : %s", filePath.string().c_str());
    ExtensionFileResult extFileResult = readExtentions(filePath);
    if (extFileResult.status == VK_FILE_SUCCESS) {
      FILE_DEBUG_PRINT("Extension(s) Loaded");
      std::string shaderF = loadShaderFromFile(filePath);
      FILE_DEBUG_PRINT("Shader Loaded");
      std::vector<uint32_t> code =
          compileGLSLToSPIRV(shaderF, get<EShLanguage>(*extFileResult.stage));
      FILE_DEBUG_PRINT("Compiled total number of uint32_t : %lu", code.size());
      sDatas.emplace_back(
          filePath.filename(),
          code,
          extFileResult.stage,
          extFileResult.platform
      );
      FILE_DEBUG_PRINT("ShaderData added");
      return VK_FILE_SUCCESS;
    }
    else {
      FILE_DEBUG_PRINT("Extension couldn't be loaded : %s ", filePath.c_str());
      return VK_FILE_NOT_A_SHADER;
    }
  }
  else {
    FILE_DEBUG_PRINT(
        "At given path : %s - No shader elements found : ERROR ",
        filePath.string().c_str()
    );
    return VK_FILE_UNKNOWN_ERROR;
  }
}

uint32_t ShaderHandler::loadShaderDataFromFolder(
    const std::filesystem::path &dirPath
)
{
  uint32_t count = 0;

  if (!std::filesystem::is_directory(dirPath)) {
    FILE_DEBUG_PRINT("The path is not a directory or does not exist.");
  }

  for (auto f : std::filesystem::directory_iterator(dirPath)) {
    std::cout << " count : " << count
              << " filename:  " << f.path().filename().string() << '\n';
    count += std::filesystem::is_regular_file(f) &&
             loadShaderDataFile(f) == VK_FILE_SUCCESS;
  }
  return const_cast<uint32_t &>(count);
}

const std::string ShaderHandler::loadShaderFromFile(
    const std::filesystem::path &shaderFile
)
{
  std::ifstream file(shaderFile);
  if (!file.is_open()) {
    throw std::runtime_error(
        std::string("Couldn't load Shader File") + shaderFile.string()
    );
  }
  return std::string(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>())
  );
}

const std::vector<uint32_t> ShaderHandler::compileGLSLToSPIRV(
    const std::string &shaderCode,
    const EShLanguage &shaderStage
)
{
  glslang::TShader                  shader(shaderStage);
  const std::array<const char *, 1> shaderStrings = {shaderCode.c_str()};

  shader.setStrings(shaderStrings.data(), shaderStrings.size());
  shader.setEnvInput(
      glslang::EShSourceGlsl,
      shaderStage,
      glslang::EShClientVulkan,
      OPENGL_CHOSEN_VERSION
  );
  shader.setEnvClient(glslang::EShClientVulkan, VULKAN_CHOSEN_VERSION);
  shader.setEnvTarget(glslang::EShTargetSpv, SPIRV_CHOSEN_VERSION);

  if (!shader.parse(GetDefaultResources(), 100, false, EShMsgDefault)) {
    FILE_DEBUG_PRINT("GLSL Parsing Failed for shader stage: %d ", shaderStage);
    FILE_DEBUG_PRINT("%s\n%s", shader.getInfoLog(), shader.getInfoDebugLog());
    return {};
  }

  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(EShMsgDefault)) {
    FILE_DEBUG_PRINT("GLSL Linking Failed : %s", program.getInfoLog());
    FILE_DEBUG_PRINT("%s", program.getInfoDebugLog());
    return {};
  }

  std::vector<uint32_t>   spirv;
  glslang::TIntermediate *intermediate = program.getIntermediate(shaderStage);
  if (!intermediate) {
    FILE_DEBUG_PRINT("Failed to get intermediate representation");
  }

  glslang::SpvOptions spvOptions;
  spv::SpvBuildLogger logger;
  glslang::GlslangToSpv(*intermediate, spirv, &logger, &spvOptions);

  if (logger.getAllMessages().size()) {
    FILE_DEBUG_PRINT("Spirv Logger : %s", logger.getAllMessages().c_str());
  }

  return spirv;
}

std::string ShaderHandler::listAllShaders() const
{
  return std::accumulate(
      sDatas.begin(),
      sDatas.end(),
      std::string(),
      [](const std::string &a, const ShaderDataFile &b) {
        return a + b.getName() + '\n';
      }
  );
}

const ShaderDataFile *ShaderHandler::getShader(
    std::string name
) const
{
  std::list<ShaderDataFile>::const_iterator result = std::find_if(
      sDatas.cbegin(),
      sDatas.cend(),
      [&name](const ShaderDataFile &a) -> bool { return a.getName() == name; }
  );
  return result != sDatas.end() ? &(*result) : nullptr;
}

[[nodiscard]] ShaderHandler::operator std::vector<VkShaderModuleCreateInfo>(
) const
{
  std::vector<VkShaderModuleCreateInfo> result;
  result.reserve(sDatas.size());
  for (const auto &a : sDatas) {
    result.push_back(static_cast<VkShaderModuleCreateInfo>(a));
  }
  return result;
}

void ShaderDataFile::performReflection() {
  SpvReflectResult result = spvReflectCreateShaderModule(
      code.size() * sizeof(uint32_t),
      code.data(),
      &reflectModule
  );

  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    FILE_DEBUG_PRINT("Failed to create SPIRV-Reflect shader module for %s", name.c_str());
    reflectionValid = false;
    return;
  }

  reflectionValid = true;

  // Enumerate descriptor sets
  result = spvReflectEnumerateDescriptorSets(&reflectModule, &reflectionData.descriptorSetCount, nullptr);
  if (result == SPV_REFLECT_RESULT_SUCCESS && reflectionData.descriptorSetCount > 0) {
    reflectionData.descriptorSets.resize(reflectionData.descriptorSetCount);
    result = spvReflectEnumerateDescriptorSets(&reflectModule, &reflectionData.descriptorSetCount, reflectionData.descriptorSets.data());

    if (result == SPV_REFLECT_RESULT_SUCCESS) {
      FILE_DEBUG_PRINT("Shader %s has %u descriptor set(s)", name.c_str(), reflectionData.descriptorSetCount);
      for (uint32_t i = 0; i < reflectionData.descriptorSetCount; ++i) {
        const SpvReflectDescriptorSet* set = reflectionData.descriptorSets[i];
        FILE_DEBUG_PRINT("  Set %u: %u binding(s)", set->set, set->binding_count);
      }
    }
  }

  // Enumerate push constants
  result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &reflectionData.pushConstantCount, nullptr);
  if (result == SPV_REFLECT_RESULT_SUCCESS && reflectionData.pushConstantCount > 0) {
    reflectionData.pushConstants.resize(reflectionData.pushConstantCount);
    result = spvReflectEnumeratePushConstantBlocks(&reflectModule, &reflectionData.pushConstantCount, reflectionData.pushConstants.data());

    if (result == SPV_REFLECT_RESULT_SUCCESS) {
      FILE_DEBUG_PRINT("Shader %s has %u push constant block(s)", name.c_str(), reflectionData.pushConstantCount);
      for (uint32_t i = 0; i < reflectionData.pushConstantCount; ++i) {
        const SpvReflectBlockVariable* block = reflectionData.pushConstants[i];
        FILE_DEBUG_PRINT("  Push constant: %s, size: %u bytes", block->name, block->size);
      }
    }
  }

  // Enumerate input variables
  result = spvReflectEnumerateInputVariables(&reflectModule, &reflectionData.inputVariableCount, nullptr);
  if (result == SPV_REFLECT_RESULT_SUCCESS && reflectionData.inputVariableCount > 0) {
    reflectionData.inputVariables.resize(reflectionData.inputVariableCount);
    result = spvReflectEnumerateInputVariables(&reflectModule, &reflectionData.inputVariableCount, reflectionData.inputVariables.data());

    if (result == SPV_REFLECT_RESULT_SUCCESS) {
      FILE_DEBUG_PRINT("Shader %s has %u input variable(s)", name.c_str(), reflectionData.inputVariableCount);
    }
  }

  // Enumerate output variables
  result = spvReflectEnumerateOutputVariables(&reflectModule, &reflectionData.outputVariableCount, nullptr);
  if (result == SPV_REFLECT_RESULT_SUCCESS && reflectionData.outputVariableCount > 0) {
    reflectionData.outputVariables.resize(reflectionData.outputVariableCount);
    result = spvReflectEnumerateOutputVariables(&reflectModule, &reflectionData.outputVariableCount, reflectionData.outputVariables.data());

    if (result == SPV_REFLECT_RESULT_SUCCESS) {
      FILE_DEBUG_PRINT("Shader %s has %u output variable(s)", name.c_str(), reflectionData.outputVariableCount);
    }
  }
}
