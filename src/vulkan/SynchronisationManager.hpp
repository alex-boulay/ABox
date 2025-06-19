#ifndef SYNCHRONISATION_MANAGER_HPP
#define SYNCHRONISATION_MANAGER_HPP

#include "MemoryWrapper.hpp"
#include <list>
#include <map>
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

class SynchronisationManager {
  std::list<SemaphoreWrapper> semaphores;
  std::list<FenceWrapper>     fences;

  std::map<std::string, SemaphoreWrapper *> semaphoresCues;
  std::map<std::string, FenceWrapper *>     fenceCues;

   public:
  SynchronisationManager() {};

  VkResult addFence(
      VkDevice    device,
      std::string name = ""
  )
  {
    VkFenceCreateInfo fci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };
    bool newFence = name == "" || !fenceCues.contains(name);
    if (newFence) {
      fences.emplace_back(device);
    }
    VkResult result = vkCreateFence(
        device,
        &fci,
        nullptr,
        newFence ? fences.back().ptr() : fenceCues.at(name)->ptr()
    );

    if (result == VK_SUCCESS) {
      std::cout << std::boolalpha << "Fence added ! new ? " << newFence
                << " name : " << name << "Fence value  "
                << (void *)fences.back().get() << " Device Value "
                << (void *)device << std::endl;
    }
    else {
      if (newFence) {
        fences.pop_back();
      }
      throw std::runtime_error("Couldn't Create Fence !");
    }

    return result;
  }

  VkResult addSemaphore(
      VkDevice    device,
      std::string name = ""
  )
  {

    VkSemaphoreCreateInfo sci{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };

    bool newSemaphore = name == "" || !semaphoresCues.contains(name);
    if (newSemaphore) {
      semaphores.emplace_back(device);
    }
    VkResult result = vkCreateSemaphore(
        device,
        &sci,
        nullptr,
        newSemaphore ? semaphores.back().ptr() : semaphoresCues.at(name)->ptr()
    );
    if (result == VK_SUCCESS) {
      std::cout << "Semaphore added !" << std::endl;
    }
    else {
      if (newSemaphore) {
        semaphores.pop_back();
      }
      throw std::runtime_error("Couldn't Create Semaphore !");
    }
    return result;
  }

  [[nodiscard]] VkFence getFence(
      std::string name
  ) const noexcept
  {
    return fenceCues.contains(name) ? fenceCues.at(name)->get()
                                    : VK_NULL_HANDLE;
  }

  void synchroniseDraw(
      VkDevice device,
      VkFence  fence
  )
  {
    // SYNCHRONISTATION --------------
    vkWaitForFences(device, 1u, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1u, &fence);
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
