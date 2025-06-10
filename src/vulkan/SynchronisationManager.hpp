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
  SynchronisationManager(
      VkDevice device
  )
  {
    addSemaphore(device);
    addSemaphore(device);
    addFence(device);
  }

  VkResult addFence(
      VkDevice device
  )
  {
    VkFenceCreateInfo fci{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u
    };
    fences.push_back(device);
    VkResult result = vkCreateFence(device, &fci, nullptr, fences.back().ptr());

    if (result == VK_SUCCESS) {
      std::cout << "Fence added !" << std::endl;
    }
    else {
      fences.pop_back();
      throw std::runtime_error("Couldn't Create Fence !");
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

    semaphores.emplace_back(device);
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
};

#endif
