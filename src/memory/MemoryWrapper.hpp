#ifndef MEMORY_WRAPPER
#define MEMORY_WRAPPER

#include "PreProcUtils.hpp"
#include <vulkan/vulkan_core.h>

#ifdef DEBUG_VK_ABOX
  #include <iostream> // Used in MACRO so no direct call but mandatory
#endif

/**
template <typename T> class MemoryWrapper {
  std::function<void()> destroy;
  T                     container;

   public:
  MemoryWrapper(
      T                     item,
      std::function<void()> destroy
  )
      : destroy(destroy)
      , container(item)
  {
  }

  T     &get() { return container; }
  T     *ptr() { return &container; }
  inline operator T() const { return container; }

  ~MemoryWrapper() { destroy(); }

  DELETE_COPY(MemoryWrapper);

};
*/
template <typename T, typename VkD, typename VkP = std::nullptr_t>
class MemoryWrapper {
  VkP                          vulkanParent;
  T                            container;
  VkD                          vulkanDestructionFunction;
  const VkAllocationCallbacks *pAllocator;

   public:
  MemoryWrapper(
      T                            item,
      VkP                          parent,
      VkD                          destructionFunction,
      const VkAllocationCallbacks *pAllocator = nullptr
  )
      : vulkanDestructionFunction(destructionFunction)
      , vulkanParent(parent)
      , container(item)
      , pAllocator(pAllocator)
  {
  }

  ~MemoryWrapper()
  {

#ifdef DEBUG_VK_ABOX
    std::cout << " ---- Destruction of Memory wrapper -- container "
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
        std::cout << " ---- Destruction of Memory wrapper -> parent state"
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
      : vulkanDestructionFunction(other.vulkanDestructionFunction)
      , vulkanParent(other.vulkanParent)
      , container(other.container)
      , pAllocator(other.pAllocator)
  {
    other.container                 = VK_NULL_HANDLE;
    other.vulkanParent              = VK_NULL_HANDLE;
    other.vulkanDestructionFunction = nullptr;
    other.pAllocator                = nullptr;
    std::cout
        << "Dangerous -> Memory Moved (reference) next values should be null\n"
        << "previous allocator value " << (void *)vulkanParent
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
              << "\nprevious allocator value " << (void *)other.vulkanParent
              << "\nprevious container value " << (void *)other.container
              << "\nnew allocator value " << (void *)vulkanParent
              << "\nnew container value " << (void *)container << std::endl;
    return *this;
  }

  T     &get() { return container; }
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

/**
#ifdef DEBUG_VK_ABOX
  #define VK_WRAPPER_DESTROY_LAMBDA(Name, DestroyFunc, dev, pAllocator)        \
    [this, dev, pAllocator]() {                                                \
      std::cout << " ---- Destruction of " << TOSTRING(Name) << " "            \
                << TOSTRING(DestroyFunc) << "\n   -> device value"             \
                << (void *)dev << " -- container " << (void *)this->get()      \
                << std::endl;                                                  \
      if (this->get() != VK_NULL_HANDLE && dev != VK_NULL_HANDLE) {            \
        DestroyFunc(dev, this->get(), pAllocator);                             \
      }                                                                        \
    }
#else
  #define VK_WRAPPER_DESTROY_LAMBDA(Name, DestroyFunc, dev, pAllocator)        \
    [this, dev, pAllocator]() {                                                \
      if (this->get() != VK_NULL_HANDLE && dev != VK_NULL_HANDLE) {            \
        DestroyFunc(dev, this->get(), pAllocator);                             \
      }                                                                        \
    }
#endif

#define DEFINE_VK_MEMORY_WRAPPER(Type, Name, DestroyFunc)                      \
  class Name##Wrapper : public MemoryWrapper<Type> {                           \
     public:                                                                   \
    Name##Wrapper(                                                             \
        VkDevice dev,                                                          \
        Type     object                         = VK_NULL_HANDLE,              \
        const VkAllocationCallbacks *pAllocator = nullptr                      \
    )                                                                          \
        : MemoryWrapper<Type>(                                                 \
              object,                                                          \
              VK_WRAPPER_DESTROY_LAMBDA(Name, DestroyFunc, dev, pAllocator)    \
          )                                                                    \
    {                                                                          \
    }                                                                          \
  };
*/
#endif // MEMORY_WRAPPER
