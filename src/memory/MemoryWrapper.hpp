#ifndef MEMORY_WRAPPER
#define MEMORY_WRAPPER

#include "PreProcUtils.hpp"
#include <vulkan/vulkan_core.h>

#include <iostream> // Used in MACRO so no direct call but mandatory

template <typename T, typename VkD, typename VkP = std::nullptr_t>
class MemoryWrapper {
  T                            container;
  VkP                          vulkanParent;
  VkD                          vulkanDestructionFunction;
  const VkAllocationCallbacks *pAllocator;

   public:
  MemoryWrapper(
      T                            item,
      VkP                          parent,
      VkD                          destructionFunction,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : container(item)
      , vulkanParent(parent)
      , vulkanDestructionFunction(destructionFunction)
      , pAllocator(pAllocator)
  {
  }

  ~MemoryWrapper()
  {

#ifdef DEBUG_VK_ABOX
    std::cout << " ---- Destruction of Memory wrapper -- container : "
              << (void *)this->get() << std::endl;
#endif

    if (this->get() != VK_NULL_HANDLE) {
      if constexpr (std::is_same_v<VkP, std::nullptr_t>) {

#ifdef DEBUG_VK_ABOX
        std::cout << " ---- Destruction of Memory wrapper -> no parent"
                  << std::endl;
#endif

        vulkanDestructionFunction(container, pAllocator);
      }
      else {

#ifdef DEBUG_VK_ABOX
        std::cout << " ---- Destruction of Memory wrapper -> parent state : "
                  << (void *)vulkanParent << std::endl;
#endif
        if (vulkanParent != VK_NULL_HANDLE) {
          vulkanDestructionFunction(vulkanParent, container, pAllocator);
        }
      }
    }
  }

  DELETE_COPY(MemoryWrapper);

  MemoryWrapper(
      MemoryWrapper &&other
  ) noexcept
      : container(other.container)
      , vulkanParent(other.vulkanParent)
      , vulkanDestructionFunction(other.vulkanDestructionFunction)
      , pAllocator(other.pAllocator)
  {
    other.container                 = VK_NULL_HANDLE;
    other.vulkanParent              = VK_NULL_HANDLE;
    other.vulkanDestructionFunction = nullptr;
    other.pAllocator                = nullptr;
    std::cout << "Dangerous -> Memory Moved (reference) \n"
              << "previous parent value " << (void *)other.vulkanParent
              << "previous container value " << (void *)other.container << "\n"
              << "current parent value " << (void *)vulkanParent
              << "previous container value " << (void *)container << std::endl;
  }

  MemoryWrapper &operator=(
      MemoryWrapper &&other
  ) noexcept
  {
    if (this != &other) {
      container                 = other.container;
      vulkanParent              = other.vulkanParent;
      vulkanDestructionFunction = other.vulkanDestructionFunction;
      pAllocator                = other.pAllocator;

      other.container                 = VK_NULL_HANDLE;
      other.vulkanParent              = VK_NULL_HANDLE;
      other.vulkanDestructionFunction = nullptr;
      other.pAllocator                = nullptr;
    }

    std::cout << "Dangerous -> Memory Moved (assigned)"
              << "\nprevious parent value " << (void *)other.vulkanParent
              << "\nprevious container value " << (void *)other.container
              << "\nnew parent value " << (void *)vulkanParent
              << "\nnew container value " << (void *)container << std::endl;
    return *this;
  }
  T      get() const { return container; }
  T     *ptr() { return &container; }
  inline operator T() const { return container; }
};

#define DEFINE_VK_MEMORY_WRAPPER_FULL(Type, Name, DestroyFunc, VkParent)       \
  class Name##Wrapper                                                          \
      : public MemoryWrapper<Type, decltype(&DestroyFunc), VkParent> {         \
     public:                                                                   \
    Name##Wrapper(                                                             \
        VkParent dev,                                                          \
        Type     object                         = VK_NULL_HANDLE,              \
        const VkAllocationCallbacks *pAllocator = nullptr                      \
    )                                                                          \
        : MemoryWrapper<Type, decltype(&DestroyFunc), VkParent>(               \
              object,                                                          \
              dev,                                                             \
              DestroyFunc,                                                     \
              pAllocator                                                       \
          )                                                                    \
    {                                                                          \
    }                                                                          \
  };
#define DEFINE_VK_MEMORY_WRAPPER(Type, Name, DestroyFunc)                      \
  DEFINE_VK_MEMORY_WRAPPER_FULL(Type, Name, DestroyFunc, VkDevice)

#define DEFINE_VK_MEMORY_WRAPPER_SOLO(Type, Name, DestroyFunc)                 \
  class Name##Wrapper : public MemoryWrapper<Type, decltype(&DestroyFunc)> {   \
     public:                                                                   \
    Name##Wrapper(                                                             \
        Type object                             = VK_NULL_HANDLE,              \
        const VkAllocationCallbacks *pAllocator = nullptr                      \
    )                                                                          \
        : MemoryWrapper<Type, decltype(&DestroyFunc)>(                         \
              object,                                                          \
              nullptr,                                                         \
              DestroyFunc,                                                     \
              pAllocator                                                       \
          )                                                                    \
    {                                                                          \
    }                                                                          \
  };
#endif // MEMORY_WRAPPER
