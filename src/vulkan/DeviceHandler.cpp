#include "DeviceHandler.hpp"
#include "CommandsHandler.hpp"
#include "Logger.hpp"
#include "ShaderHandler.hpp"
#include "vectorUtils.hpp"
#include <bitset>
#include <climits>
#include <cstdint>
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace ABox_Utils {

std::set<const char *> requiredDeviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
std::set<const char *> optionalDeviceExtentions{
    "VK_KHR_portability_subset",
};

std::set<VkQueueFlags> necessaryDeviceQueueFamilyFLags{
    VK_QUEUE_GRAPHICS_BIT,
    VK_QUEUE_COMPUTE_BIT
};

bool hasGraphicQueue(VkQueueFamilyProperties qfp)
{
  return qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

bool hasComputeQueue(VkQueueFamilyProperties qfp)
{
  return qfp.queueFlags & VK_QUEUE_COMPUTE_BIT;
}

bool supportsPresentation(
    VkPhysicalDevice pD,
    VkSurfaceKHR     surface,
    uint32_t         qFamIndex
)
{
  VkBool32 support;
  VkResult result =
      vkGetPhysicalDeviceSurfaceSupportKHR(pD, qFamIndex, surface, &support);
  if (result != VK_SUCCESS) {
    LOG_ERROR(
        "Vulkan"
    ) << "Error Querying Physical Device Support for KHR Surfaces!";
    LOG_ERROR("Vulkan") << "Phy " << (void *)pD << " - Surface "
                        << (void *)surface << " - qFamIndex " << qFamIndex;
    LOG_ERROR("Vulkan") << "VK_ERROR Value: " << result;
    return false;
  }
  return support;
}

bool isValidQueueFamily(VkQueueFamilyProperties qfp)
{
  for (auto n : necessaryDeviceQueueFamilyFLags) {
    if (!(n & qfp.queueFlags)) {
      return false;
    }
  }
  return true;
}

uint32_t queueFamilyQueueCount(VkQueueFamilyProperties qfp)
{
  return std::bitset<sizeof(qfp.queueFlags) * CHAR_BIT>(qfp.queueFlags).count();
}

struct ExtensionSupport {
  std::vector<const char *> enabled; // list to pass to vkCreateDevice
  bool allRequired = true;           // false if any required ext is missing
};

ExtensionSupport filterDeviceExtensions(VkPhysicalDevice phys)
{
  uint32_t count = 0;
  vkEnumerateDeviceExtensionProperties(phys, nullptr, &count, nullptr);
  std::vector<VkExtensionProperties> avail(count);
  vkEnumerateDeviceExtensionProperties(phys, nullptr, &count, avail.data());

  auto hasExt = [&](const char *name) {
    for (auto &e : avail) {
      if (strcmp(e.extensionName, name) == 0) {
        return true;
      }
    }
    return false;
  };

  ExtensionSupport result;

  // Required
  for (auto *name : requiredDeviceExtensions) {
    if (hasExt(name)) {
      result.enabled.push_back(name);
    }
    else {
      LOG_ERROR("Vulkan") << "Missing REQUIRED device extension: " << name;
      result.allRequired = false;
    }
  }

  // Optional
  for (auto *name : optionalDeviceExtentions) {
    if (hasExt(name)) {
      result.enabled.push_back(name);
      LOG_INFO("Vulkan") << "Enabling optional extension: " << name;
    }
    else {
      LOG_WARN("Vulkan") << "Optional extension not present: " << name;
    }
  }
  return result;
}

uint32_t DeviceHandler::listQueueFamilies()
{
  uint32_t queueCount;
  LOG_DEBUG("Device") << "Listing Queue Families:";
  for (uint16_t i = 0; i < phyDevices.size(); i++) {
    LOG_DEBUG("Device") << "Physical Device #" << i;
    vkGetPhysicalDeviceQueueFamilyProperties(
        phyDevices.at(i),
        &queueCount,
        nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        phyDevices.at(i),
        &queueCount,
        queueFamilies.data()
    );
    for (uint16_t qi = 0; qi < queueFamilies.size(); qi++) {
      const auto &qf = queueFamilies.at(qi);
      LOG_DEBUG("Device") << "  QueueFamily #" << qi
                          << " - Count: " << qf.queueCount
                          << vkQueueFlagSS(qf.queueFlags).str();
      LOG_DEBUG("Device") << "  Is valid QueueFamily: " << std::boolalpha
                          << bool(isValidQueueFamily(qf));
    }
  }
  return queueCount;
}

DeviceHandler::DeviceHandler(VkInstance instance)
{
  uint32_t deviceCount = 0u;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  LOG_INFO("Device") << "Total number of physical devices: " << deviceCount;
  phyDevices.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, phyDevices.data());
}

/**
VkResult DeviceHandler::DeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0u;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
deviceExtensions.end()); for (const auto &extension : availableExtensions)
    {
        LOG_DEBUG("Vulkan") << "Dev extension: " << extension.extensionName
                            << " - Version: " << extension.specVersion;
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty() ? VK_SUCCESS :
VK_ERROR_EXTENSION_NOT_PRESENT;
}*/

VkResult DeviceHandler::listPhysicalDevices() const
{
  uint32_t                   index   = 0;
  VkPhysicalDeviceProperties phyProp = {};
  VkPhysicalDeviceFeatures   phyFeat = {};
  for (auto physical : phyDevices) {
    vkGetPhysicalDeviceProperties(physical, &phyProp);
    LOG_DEBUG("Device") << "Device #" << index << ": " << phyProp.deviceName
                        << " (API " << VK_VERSION_MAJOR(phyProp.apiVersion)
                        << "." << VK_VERSION_MINOR(phyProp.apiVersion) << ")";
    vkGetPhysicalDeviceFeatures(physical, &phyFeat);
    index++;
  }
  return index > 0 ? VK_SUCCESS : VK_ERROR_DEVICE_LOST;
}

void DeviceHandler::waitIdle()
{
  for (auto &dev : devices) {
    vkDeviceWaitIdle(dev.getDevice().get());
  }
}

std::unordered_map<QueueRole, uint32_t>
    DeviceHandler::loadNecessaryQueueFamilies(
        uint32_t     phyDev,
        VkSurfaceKHR surface
    )
{
  std::unordered_map<QueueRole, uint32_t> result;
  uint32_t                                queueCount = 0;
  VkPhysicalDevice                        pPD        = phyDevices.at(phyDev);
  vkGetPhysicalDeviceQueueFamilyProperties(pPD, &queueCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(
      pPD,
      &queueCount,
      queueFamilies.data()
  );
  for (uint32_t i = 0; i < queueCount; i++) {
    if (hasGraphicQueue(queueFamilies.at(i))) {
      result[QueueRole::Graphics] = i;
      break;
    }
  }
  for (uint32_t i = 0; i < queueCount; i++) {
    if (supportsPresentation(pPD, surface, i)) {
      result[QueueRole::Present] = i;
      break;
    }
  }
  return result;
}

std::set<uint32_t> DeviceHandler::getQueueFamilyIndices(QueueFamilyIndices fi)
{
  std::set<uint32_t> indices;
  for (auto [key, val] : fi) {
    indices.insert(val);
  }
  return indices;
}

std::vector<uint32_t>
    DeviceHandler::listQueueFamilyIndices(QueueFamilyIndices fi)
{
  auto s = getQueueFamilyIndices(fi);
  return std::vector<uint32_t>(s.cbegin(), s.cend());
}

VkResult DeviceHandler::addLogicalDevice(
    VkSurfaceKHR surface,
    std::string  name,
    uint32_t     index
)
{
  if (surface == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Given Surface is not initialised (or has already been freed)."
    );
  }
  const float queuePriority = 1.0f;

  VkPhysicalDevice                        phydev = phyDevices[index];
  std::unordered_map<QueueRole, uint32_t> fIndices =
      loadNecessaryQueueFamilies(index, surface);

  std::vector<VkDeviceQueueCreateInfo> qCI;

  LOG_DEBUG("Device") << "Creating queue create infos";
  for (uint32_t famIndex : listQueueFamilyIndices(fIndices)) {
    qCI.push_back(
        VkDeviceQueueCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = 0u,
            .queueFamilyIndex = famIndex,
            .queueCount       = 1u,
            .pQueuePriorities = &queuePriority
        }
    );
  }
  LOG_DEBUG("Device") << "Preparing logical device creation";
  std::vector<const char *> devExtVect = filterDeviceExtensions(phydev).enabled;
  listQueueFamilies();
  VkDeviceCreateInfo devInfo{
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext                   = nullptr,
      .flags                   = 0u,
      .queueCreateInfoCount    = static_cast<uint32_t>(qCI.size()),
      .pQueueCreateInfos       = qCI.data(),
      .enabledLayerCount       = 0u,      // should not be used
      .ppEnabledLayerNames     = nullptr, // should not be used
      .enabledExtensionCount   = static_cast<uint32_t>(devExtVect.size()),
      .ppEnabledExtensionNames = devExtVect.data(),
      .pEnabledFeatures        = nullptr,
  };

  VkDevice dev;
  VkResult res = vkCreateDevice(phydev, &devInfo, nullptr, &dev);

  if (res == VK_SUCCESS) {
    LOG_INFO("Device") << "Logical Device Assignment success!";
    devices.emplace_back(dev, phydev, fIndices);
    if (deviceNames.contains(name)) {
      LOG_WARN("Device") << "Overlapping device name: " << name << " - Device: "
                         << (void *)deviceNames.at(name)->getDevice()
                         << " Might not have been freed";
    }
    deviceNames[name] = &devices.back();
  }
  else {
    LOG_ERROR("Device") << "Logical Device Assignment Failure, Result Code: "
                        << res;
  }
  LOG_DEBUG("Device") << "fIndices size: " << fIndices.size();
  for (const auto &[key, value] : fIndices) {
    LOG_DEBUG("Device") << "key " << static_cast<int>(key) << " val " << value;
  }
  vkGetDeviceQueue(
      dev,
      fIndices.at(QueueRole::Graphics),
      0,
      &devices.back().graphicsQueue
  );

  vkGetDeviceQueue(
      dev,
      fIndices.at(QueueRole::Present),
      0u,
      &(devices.back().presentQueue)
  );

  return res;
} // namespace ABox_Utils

uint32_t rateDeviceSuitability(
    VkPhysicalDeviceProperties deviceProperties,
    VkPhysicalDeviceFeatures   deviceFeatures
)
{
  uint32_t score = 0;

  // Les carte graphiques dédiées ont un énorme avantage en terme de
  // performances
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  else if (deviceProperties.deviceType ==
           VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    score += 200;
  }

  // La taille maximale des textures affecte leur qualité
  score += deviceProperties.limits.maxImageDimension2D;

  // L'application (fictive) ne peut fonctionner sans les geometry shaders
  if (!deviceFeatures.geometryShader) {
    return 0;
  }

  return score;
}

uint32_t DeviceHandler::findBestPhysicalDevice()
{
  // Score maping to physical device index
  std::multimap<uint32_t, uint32_t> candidates;

  VkPhysicalDeviceProperties phyProp = {};
  VkPhysicalDeviceFeatures   phyFeat = {};
  for (uint32_t devIndex = 0; devIndex < phyDevices.size(); devIndex++) {
    vkGetPhysicalDeviceProperties(phyDevices.at(devIndex), &phyProp);
    vkGetPhysicalDeviceFeatures(phyDevices.at(devIndex), &phyFeat);
    uint32_t score = rateDeviceSuitability(phyProp, phyFeat);
    candidates.insert(std::make_pair(score, devIndex));
    LOG_DEBUG("Device") << "Score: " << score << "\tdevIndex: " << devIndex;
  }

  if (candidates.rbegin()->first > 0) {
    LOG_INFO("Device") << "Physical Device selected is PhyDevice #"
                       << candidates.rbegin()->second;
    return candidates.rbegin()->second;
  }
  else {
    throw std::runtime_error(
        "None of the installed GPU are compatible with the Application ! \n"
    );
  }
  return 0;
}

VkResult DeviceHandler::addLogicalDevice(VkSurfaceKHR surface)
{
  return addLogicalDevice(surface, "main", findBestPhysicalDevice());
}

DeviceBoundElements *DeviceHandler::getDBE(uint32_t index)
{
  if (index >= devices.size()) {
    return nullptr;
  }

  auto it = devices.begin();
  std::advance(it, index);
  return &(*it);
}

DeviceBoundElements *DeviceHandler::getDBE(std::string name)
{
  return deviceNames.contains(name) ? deviceNames.at(name) : nullptr;
}

VkDevice DeviceHandler::getDevice(uint32_t index)
{
  return getDBE(index)->getDevicePtr()->get();
}

VkResult DeviceHandler::addSwapchain(
    uint32_t      width,
    uint32_t      height,
    VkSurfaceKHR *surface,
    uint_fast8_t  devIndex
)
{
  if (devices.size() <= devIndex) {
    return VK_ERROR_DEVICE_LOST;
  }

  DeviceBoundElements *devicePtr = getDBE(devIndex);
  LOG_DEBUG("Device") << "DBE mapping";
  LOG_DEBUG("Device") << "DBE Physical: "
                      << (void *)devicePtr->getPhysicalDevice();
  LOG_DEBUG("Device") << "DBE logical: " << (void *)getDevice(devIndex);
  LOG_DEBUG("Device") << "DBE surface: " << (void *)surface;
  LOG_DEBUG("Device") << "DBE rQDI: " << std::boolalpha
                      << devicePtr->getFamilyQueueIndices().at(
                             QueueRole::Present
                         );
  LOG_DEBUG("Device") << "DBE gQDI: " << std::boolalpha
                      << devicePtr->getFamilyQueueIndices().at(
                             QueueRole::Graphics
                         );

  getDBE(devIndex)->swapchain.emplace(
      devicePtr->getPhysicalDevice(),
      surface,
      getDevice(devIndex),
      devicePtr->getFamilyQueueIndices().at(QueueRole::Present),
      devicePtr->getFamilyQueueIndices().at(QueueRole::Graphics),
      width,
      height
  );
  LOG_INFO("Device") << "SwapchainMapping done";

  return VK_SUCCESS;
}

VkResult DeviceHandler::addGraphicsPipeline(
    uint32_t                         deviceIndex,
    const std::list<ShaderDataFile> &shaderFiles
)
{
  DeviceBoundElements *dbe = getDBE(deviceIndex);
  if (dbe && dbe->swapchains.front() {
    LOG_INFO("Pipeline") << "Loading Graphics Pipeline with "
                         << shaderFiles.size() << " shaders";

    // Create main graphics pipeline in PipelineManager
    dbe->pipelineManager.createGraphicsPipeline(
        getDevice(deviceIndex),
        "main",
        dbe->swapchains.front(),
        shaderFiles,
        true // setAsMain = true
    );
    return VK_SUCCESS;
  }
  LOG_ERROR("Pipeline"
  ) << "Failed to initialise Graphics Pipeline in the Device Manager";
  return VK_ERROR_INITIALIZATION_FAILED;
};

//------DISPLAY FUNCTIONS --- Maybe Need to opacity----//
OSTREAM_OP(const VkPhysicalDeviceType &phyT)
{
  switch (phyT) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return os << "other type GPU";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return os << "integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return os << "discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return os << "virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return os << "CPU";
    default: return os << " undefined GPU type";
  }
}

OSTREAM_OP(const VkPhysicalDeviceProperties &phyP)
{
  os << "---------- Physical Device Properties ----------\n";
  os << "\t API version : " << phyP.apiVersion << '\n';
  os << "\t driver version : " << phyP.driverVersion << '\n';
  os << "\t vendor ID : " << phyP.vendorID << '\n';
  os << "\t device ID : " << phyP.deviceID << '\n';
  os << "\t device type : " << phyP.deviceType << '\n';
  os << "\t device name : " << phyP.deviceName << '\n';
  return os;
}

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag)
{
  std::stringstream   ss;
  const uint16_t      f_size = sizeof(VkQueueFlags) * CHAR_BIT;
  std::bitset<f_size> bits(flag);
  ss << "\nVkQueue Flags Bits : ";
  for (uint32_t i = 0; i < f_size; i++) {
    ss << bits[f_size - 1 - i] << (i % CHAR_BIT == (CHAR_BIT - 1) ? " " : "");
  }
  ss << (bool(flag & VK_QUEUE_GRAPHICS_BIT) ? "\nVK_QUEUE_GRAPHICS_BIT - 0x1"
                                            : "")
     << (bool(flag & VK_QUEUE_COMPUTE_BIT) ? "\nVK_QUEUE_COMPUTE_BIT - 0x2"
                                           : "")
     << (bool(flag & VK_QUEUE_TRANSFER_BIT) ? "\nVK_QUEUE_TRANSFER_BIT - 0x4"
                                            : "")
     << (bool(flag & VK_QUEUE_SPARSE_BINDING_BIT)
             ? "\nVK_QUEUE_SPARSE_BINDING_BIT - 0x8"
             : "")
     << (bool(flag & VK_QUEUE_PROTECTED_BIT) ? "\nVK_QUEUE_PROTECTED_BIT - 0x10"
                                             : "")
     << (bool(flag & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
             ? "\nVK_QUEUE_VIDEO_DECODE_BIT_KHR - 0x20"
             : "")
     << (bool(flag & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
             ? "\nVK_QUEUE_VIDEO_ENCODE_BIT_KHR - 0x40"
             : "")
     << (bool(flag & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
             ? "\nVK_QUEUE_OPTICAL_FLOW_BIT_NV - 0x100"
             : "")
     << (static_cast<uint32_t>(flag) == VK_QUEUE_FLAG_BITS_MAX_ENUM
             ? "\nVK_QUEUE_FLAG_BITS_MAX_ENUM - 0x7FFFFFFF"
             : "");
  return ss;
}

OSTREAM_OP(const VkExtent3D &ext)
{
  os << " Width : " << ext.width << " - Height : " << ext.height
     << " - Depth : " << ext.depth;
  return os;
}

OSTREAM_OP(const VkQueueFamilyProperties &prop)
{
  os << "VkQueueFamilyProperties : {\n\t queueCount : " << prop.queueCount
     << ",\n\t timeStampValidBits : " << prop.timestampValidBits
     << ",\n\t mimImageTransferGranularity : "
     << prop.minImageTransferGranularity
     << ",\n\t queueFlags : " << vkQueueFlagSS(prop.queueFlags).str() << "\n};";
  return os;
}

} // namespace ABox_Utils
