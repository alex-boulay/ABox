#ifndef DEVICE_HANDLER_HPP
#define DEVICE_HANDLER_HPP
#include "GraphicsPipeline.hpp"
#include "PreProcUtils.hpp"
#include "ShaderHandler.hpp"
#include "SwapchainManager.hpp"
#include <cstdint>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ABox_Utils
{

/**
 * @struct QueueFamilyIndices
 * @brief represent optional queue family indices
 */
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicQueueIndex;
    std::optional<uint32_t> presentQueueIndex;
};

/**
 * @struct DeviceBoundElements
 * @brief Represents elements bounded to the Logical Device
 */
struct DeviceBoundElements
{
    VkPhysicalDevice                                physical;
    QueueFamilyIndices                              fIndices;
    std::optional<SwapchainManager>                 swapchain;
    std::optional<GraphicsPipeline>                 graphicsppl;
    std::unordered_map<std::string, VkShaderModule> loadedShaders;
};

/**
 * @class DeviceHandler
 * @brief Handle specifics to logical devices and their different bindings
 */
class DeviceHandler
{
    std::vector<VkPhysicalDevice>                          phyDevices;
    std::vector<VkDevice>                                  devices;
    std::unordered_map<uint_fast16_t, DeviceBoundElements> deviceMap;

    std::set<uint32_t>                                     getQueueFamilyIndices(QueueFamilyIndices fi);
    std::vector<uint32_t>                                  listQueueFamilyIndices(QueueFamilyIndices fi);

public:
    DeviceHandler(DeviceHandler &&other) noexcept
        : phyDevices(other.phyDevices)
        , devices(other.devices)
    {
        for (auto &a : other.deviceMap)
        {
            deviceMap[a.first] = {
                .physical      = a.second.physical,
                .fIndices      = a.second.fIndices,
                .swapchain     = std::move(a.second.swapchain),
                .graphicsppl   = std::move(a.second.graphicsppl),
                .loadedShaders = std::move(a.second.loadedShaders)    // Wrap ShaderModules into own class ?
            };
            a.second.graphicsppl.reset();
            a.second.swapchain.reset();
        }
    }

    ~DeviceHandler()
    {
        std::cout << " Destruction : Device Map : Size " << deviceMap.size() << std::endl;
        for (auto &a : deviceMap)
        {
            std::cout << "Device : " << a.first << " - Shaders " << a.second.loadedShaders.size() << std::endl;
            for (auto &pair : a.second.loadedShaders)
            {
                std::cout << "Shader name " << pair.first << std::endl;
                vkDestroyShaderModule(devices[a.first], pair.second, nullptr);
            }
            // a.second.loadedShaders.clear();
        }
        std::cout << "Clearing device map" << std::endl;
        for (auto &a : deviceMap)
        {
            if (a.second.swapchain.has_value())
            {
                std::cout << "Removing swapchainandler from device." << std::endl;
                a.second.swapchain.reset();
            }
        }
        // deviceMap.clear();
        for (auto &dev : devices)
        {
            if (dev != VK_NULL_HANDLE)
            {
                std::cout << "Destroying device :" << (void *) dev << std::endl;
                vkDestroyDevice(dev, nullptr);
                std::cout << "Device destroyed : " << (dev == VK_NULL_HANDLE);
            }
        }
        // devices.clear();
    }

    DeviceHandler &operator=(DeviceHandler &&other) noexcept
    {
        if (this != &other)
        {
            // Copy assignment: copy the members from `other` to `this`
            phyDevices = other.phyDevices;
            devices    = other.devices;

            other.phyDevices.clear();
            other.devices.clear();
            for (auto &a : other.deviceMap)
            {
                deviceMap[a.first] = {.physical      = a.second.physical,
                                      .fIndices      = a.second.fIndices,
                                      .swapchain     = std::move(a.second.swapchain),
                                      .graphicsppl   = std::move(a.second.graphicsppl),
                                      .loadedShaders = std::move(a.second.loadedShaders)};
                a.second.graphicsppl.reset();
                a.second.swapchain.reset();
            }
        }
        return *this;
    }

    VkResult           listPhysicalDevices() const;

    /**
     * @brief add a Logical device while guessing which Physical Device is the
     * best suited to do the job
     *
     * @param VkSurfaceKHR surface the surface which will bind presentation
     * @return VkResult : VK_SUCCESS if succeded else a corresponding error
     * value
     */
    VkResult           addLogicalDevice(VkSurfaceKHR surface);
    /**
     *
     * @param uint32_t index the physical device index to select
     * @param[[VkSurfaceKHR] surface the surface which will bind presentation
     * @return a VkResult if VK_SUCCESS if succeded else a corresponding error
     * value
     */
    VkResult           addLogicalDevice(uint32_t index, VkSurfaceKHR surface);

    // TODO : VkResult clear();
    uint32_t           listQueueFamilies();
    VkResult           DeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices loadNecessaryQueueFamilies(uint32_t phyDev, VkSurfaceKHR surface);

    uint32_t           findBestPhysicalDevice();

    DeviceHandler() {};
    DeviceHandler(VkInstance instance);

    VkDevice   *getDevice(uint32_t index);

    inline bool hasDevice(uint32_t index) const { return index < devices.size(); }

    // DeviceBoundElements getBoundElements(uint_fast16_t devIndex) const;
    VkResult    addSwapchain(uint32_t width, uint32_t height, VkSurfaceKHR *surface, uint_fast8_t devIndex);

    std::pair<VkResult, VkShaderModule> loadShader(uint_fast16_t deviceIndex, const ShaderDataFile &sdf);

    /**
     * @brief add a GraphicsPipeline to a LogicalDevice which must have a
     * swapchain
     */
    VkResult addGraphicsPipeline(uint32_t deviceIndex, const std::list<ShaderDataFile> &shaderFiles);

    // No copy
    DELETE_COPY(DeviceHandler)
};

std::stringstream vkQueueFlagSS(const VkQueueFlags &flag);
OSTREAM_OP(const VkQueueFamilyProperties &prop);
OSTREAM_OP(const VkExtent3D &ext);
OSTREAM_OP(const VkPhysicalDeviceProperties &phyP);
OSTREAM_OP(const VkPhysicalDeviceType &phyT);
}    // namespace ABox_Utils
#endif
