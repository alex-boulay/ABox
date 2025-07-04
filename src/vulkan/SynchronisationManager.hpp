#ifndef SYNCHRONISATION_MANAGER_HPP
#define SYNCHRONISATION_MANAGER_HPP

#include "MemoryWrapper.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <list>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#define INFLIGHT_NUMBER_OF_ELEMENTS 2

DEFINE_VK_MEMORY_WRAPPER(
    VkSemaphore,
    Semaphore,
    vkDestroySemaphore
)

DEFINE_VK_MEMORY_WRAPPER(
    VkFence,
    Fence,
    vkDestroyFence
)

class FrameSyncObject {

   public:
  SemaphoreWrapper imageOk;
  SemaphoreWrapper renderEnd;
  FenceWrapper     inFlight;

  FrameSyncObject(
      VkDevice dev
  )
      : imageOk(dev)
      , renderEnd(dev)
      , inFlight(dev)
  {

    VkFenceCreateInfo fci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VkResult result = vkCreateFence(dev, &fci, nullptr, inFlight.ptr());

    if (result == VK_SUCCESS) {
      std::cout << "Infight Fence added ! " << (void *)inFlight << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't create Fence");
    }
    VkSemaphoreCreateInfo sci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };

    result = vkCreateSemaphore(dev, &sci, nullptr, imageOk.ptr());
    if (result == VK_SUCCESS) {
      std::cout << "Image Ok Semaphore added !" << (void *)imageOk << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't Create Semaphore !");
    }
    result = vkCreateSemaphore(dev, &sci, nullptr, renderEnd.ptr());

    if (result == VK_SUCCESS) {
      std::cout << "RenderEnd Semaphore added !" << (void *)renderEnd
                << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't Create Semaphore !");
    }
  }
};
class FrameSyncArray {
  std::vector<FrameSyncObject> framesSync;
  int16_t                      frameIndex = 0u;

   public:
  FrameSyncArray(
      VkDevice device,
      int32_t  arraySize = INFLIGHT_NUMBER_OF_ELEMENTS
  )
  {
    constexpr int32_t maxi16 =
        static_cast<int32_t>(std::numeric_limits<int16_t>::max());
    if (arraySize != std::clamp(arraySize, 0, maxi16)) {
      std::stringstream error_s;
      error_s << " Invalid size for Frame Synchronisation "
              << "Array (FrameSyncArray) : " << arraySize
              << " - whistl min = 0 and max = " << maxi16 << std::endl;
      throw std::invalid_argument(error_s.str());
    }

    for (uint32_t a = 0; a < arraySize; a++) {
      framesSync.emplace_back(device);
    }
  }
  FrameSyncObject *getFrameSyncObject(
      uint32_t index
  )
  {
    return index < framesSync.size() ? &framesSync.at(index) : nullptr;
  }

  FrameSyncObject *getFrameSyncObject()
  {
    return getFrameSyncObject(frameIndex);
  }

  uint32_t size() const { return static_cast<uint32_t>(framesSync.size()); }
  int16_t  incrementFrameIndex()
  {
    return (frameIndex = ((frameIndex + 1) % framesSync.size()));
  }
  int16_t getFrameIndex() const { return frameIndex; }
  void    resetFrameIndex() { frameIndex = 0; }

  void waitAndReset(
      VkDevice device,
      uint64_t time = UINT64_MAX
  )
  {
    vkWaitForFences(
        device,
        1,
        framesSync.at(frameIndex).inFlight.ptr(),
        VK_TRUE,
        time
    );
    vkResetFences(device, 1, framesSync.at(frameIndex).inFlight.ptr());
  }
};

class SynchronisationManager {
  std::list<SemaphoreWrapper> semaphores;
  std::list<FenceWrapper>     fences;

   public:
  SynchronisationManager() {}

  VkResult addFence(
      VkDevice device
  )
  {
    VkFenceCreateInfo fci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    fences.emplace_back(device);
    VkResult result = vkCreateFence(device, &fci, nullptr, fences.back().ptr());

    if (result == VK_SUCCESS) {
      std::cout << "Semaphore added !" << std::endl;
    }
    else {
      fences.pop_back();
      throw std::runtime_error("Couldn't create Fence");
    }

    return result;
  }

  VkResult addSemaphore(
      VkDevice device
  )
  {

    VkSemaphoreCreateInfo sci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };

    VkResult result =
        vkCreateSemaphore(device, &sci, nullptr, semaphores.back().ptr());
    if (result == VK_SUCCESS) {
      std::cout << "Semaphore added !" << std::endl;
    }
    else {
      semaphores.pop_back();
      throw std::runtime_error("Couldn't Create Semaphore !");
    }
    return result;
  }

  /**
    return result;
  }*/
};

#endif
