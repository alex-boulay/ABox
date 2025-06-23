#ifndef SYNCHRONISATION_MANAGER_HPP
#define SYNCHRONISATION_MANAGER_HPP

#include "MemoryWrapper.hpp"
#include <deque>
#include <list>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

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
  SemaphoreWrapper imageOk;
  SemaphoreWrapper renderEnd;
  FenceWrapper     inFlight;

   public:
  FrameSyncObject(
      VkDevice dev
  )
      : imageOk(dev)
      , renderEnd(dev)
      , inFlight(dev)
  {
    VkSemaphoreCreateInfo sci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };

    VkResult result = vkCreateSemaphore(dev, &sci, nullptr, imageOk.ptr());
    if (result == VK_SUCCESS) {
      std::cout << "Semaphore added !" << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't Create Semaphore !");
    }
    result = vkCreateSemaphore(dev, &sci, nullptr, renderEnd.ptr());

    if (result == VK_SUCCESS) {
      std::cout << "Semaphore added !" << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't Create Semaphore !");
    }

    VkFenceCreateInfo fci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    result = vkCreateFence(dev, &fci, nullptr, inFlight.ptr());

    if (result == VK_SUCCESS) {
      std::cout << "Infight Fence added !" << std::endl;
    }
    else {
      throw std::runtime_error("Couldn't create Fence");
    }
  }
};

class FrameSyncManager {
  std::deque<FrameSyncObject> framesSync;

   public:
  FrameSyncManager(
      VkDevice device,
      uint32_t size
  )
      : framesSync(size, FrameSyncObject(device))
  {
  }

  FrameSyncObject *getFrameSyncObject(
      uint32_t index
  )
  {
    return index < framesSync.size() ? &framesSync.at(index) : nullptr;
  }
};

class SynchronisationManager {
  std::list<SemaphoreWrapper> semaphores;
  std::list<FenceWrapper>     fences;

   public:
  SynchronisationManager() {};

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

  void synchroniseDraw(
      VkDevice device,
      VkFence *fence
  )
  {
    // In case of frames need to no print every frame
    std::cout << "Fence " << (void *)fence << "\t Device " << (void *)device
              << std::endl;

    // SYNCHRONISTATION --------------

    vkWaitForFences(device, 1u, fence, VK_TRUE, UINT64_MAX);

    std::cout << "Waiting fence " << std::endl;

    vkResetFences(device, 1u, fence);

    std::cout << "Reset Fence " << std::endl;
  }
  /**
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);
    // SUBMIT -----------
    VkSemaphore waitSemaphores[]   = {imageAvailableSemaphore};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSubmitInfo submitInfo{
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 1u,
        .pWaitSemaphores      = waitSemaphores,
        .pWaitDstStageMask    = waitStages,
        .commandBufferCount   = 1u,
        .pCommandBuffers      = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = signalSemaphores,
    };
    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
    return result;
  }*/
};

#endif
