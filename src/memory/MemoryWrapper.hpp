#ifndef MEMORY_WRAPPER
#define MEMORY_WRAPPER

#include "Logger.hpp"
#include "PreProcUtils.hpp"
#include <vulkan/vulkan_core.h>

#include <iostream> // Used in MACRO so no direct call but mandatory
#include <typeinfo> // For typeid

template <typename T, typename VkD, typename VkP = std::nullptr_t>
class MemoryWrapper {
  T                            container;
  VkP                          vulkanParent;
  VkD                          vulkanDestructionFunction;
  const VkAllocationCallbacks *pAllocator;

  void Destroy()
  {
    if constexpr (std::is_same_v<VkP, std::nullptr_t>) {
      LOG_DEBUG("Memory") << " ---- Destruction of Memory wrapper ["
                          << typeid(T).name() << " | no parent] -- container: "
                          << (void *)this->get();
    }
    else {
      LOG_DEBUG("Memory") << " ---- Destruction of Memory wrapper ["
                          << typeid(T).name()
                          << " | parent: " << typeid(VkP).name()
                          << "] -- container: " << (void *)this->get()
                          << " -- parent: " << (void *)vulkanParent;
    }

    if (this->get() != VK_NULL_HANDLE) {
      if constexpr (std::is_same_v<VkP, std::nullptr_t>) {
        vulkanDestructionFunction(container, pAllocator);
      }
      else {
        if (vulkanParent != VK_NULL_HANDLE) {
          vulkanDestructionFunction(vulkanParent, container, pAllocator);
        }
      }
    }
  }

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
    if constexpr (std::is_same_v<VkP, std::nullptr_t>) {
      LOG_DEBUG("Memory") << " ++++ Construction of Memory wrapper ["
                          << typeid(T).name() << "] (no parent) -- container: "
                          << (void *)this->get();
    }
    else {
      LOG_DEBUG("Memory") << " ++++ Construction of Memory wrapper ["
                          << typeid(T).name()
                          << " | parent: " << typeid(VkP).name()
                          << "] -- container: " << (void *)this->get()
                          << " -- parent: " << (void *)vulkanParent;
    }
  }

  ~MemoryWrapper() { Destroy(); }

  DELETE_COPY(MemoryWrapper);

  MemoryWrapper(MemoryWrapper &&other) noexcept
      : container(other.container)
      , vulkanParent(other.vulkanParent)
      , vulkanDestructionFunction(other.vulkanDestructionFunction)
      , pAllocator(other.pAllocator)
  {
    other.container                 = VK_NULL_HANDLE;
    other.vulkanParent              = VK_NULL_HANDLE;
    other.vulkanDestructionFunction = nullptr;
    other.pAllocator                = nullptr;

    if constexpr (std::is_same_v<VkP, std::nullptr_t>) {
      LOG_DEBUG("Memory") << "Dangerous -> Memory Moved [" << typeid(T).name()
                          << " | no parent] (move constructor)\n"
                          << "previous container: " << (void *)other.container
                          << "\n"
                          << "current container: " << (void *)container;
    }
    else {
      LOG_DEBUG("Memory") << "Dangerous -> Memory Moved [" << typeid(T).name()
                          << " | parent: " << typeid(VkP).name()
                          << "] (move constructor)\n"
                          << "previous parent: " << (void *)other.vulkanParent
                          << " previous container: " << (void *)other.container
                          << "\n"
                          << "current parent: " << (void *)vulkanParent
                          << " current container: " << (void *)container;
    }
  }

  MemoryWrapper &operator=(MemoryWrapper &&other) noexcept
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

    if constexpr (std::is_same_v<VkP, std::nullptr_t>) {
      LOG_DEBUG("Memory") << "Dangerous -> Memory Moved [" << typeid(T).name()
                          << " | no parent] (move assignment)\n"
                          << "previous container: " << (void *)other.container
                          << "\n"
                          << "new container: " << (void *)container;
    }
    else {
      LOG_DEBUG("Memory") << "Dangerous -> Memory Moved [" << typeid(T).name()
                          << " | parent: " << typeid(VkP).name()
                          << "] (move assignment)\n"
                          << "previous parent: " << (void *)other.vulkanParent
                          << " previous container: " << (void *)other.container
                          << "\n"
                          << "new parent: " << (void *)vulkanParent
                          << " new container: " << (void *)container;
    }
    return *this;
  }
  T      get() const { return container; }
  T     *ptr() { return &container; }
  inline operator T() const { return container; }

  void swap(const T &item)
  {
    Destroy();
    container = item;
  }
  bool empty() const { return container != VK_NULL_HANDLE; }
};

#define DEFINE_VK_MEMORY_WRAPPER_FULL(Type, Name, DestroyFunc, VkParent)       \
  class Name##Wrapper                                                          \
      : public MemoryWrapper<Type, decltype(&DestroyFunc), VkParent> {         \
     public:                                                                   \
    Name##Wrapper(                                                             \
        VkParent                     dev,                                      \
        Type                         object     = VK_NULL_HANDLE,              \
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
        Type                         object     = VK_NULL_HANDLE,              \
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
