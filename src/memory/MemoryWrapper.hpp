#ifndef MEMORY_WRAPPER
#define MEMORY_WRAPPER

#include "PreProcUtils.hpp"
#include <functional>
#ifdef DEBUG_VK_ABOX
  #include <iostream> // Used in MACRO so no direct call but mandatory
#endif

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
  DELETE_MOVE(MemoryWrapper);
};

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

#endif // MEMORY_WRAPPER
