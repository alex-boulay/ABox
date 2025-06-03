#ifndef MEMORY_WRAPPER
#define MEMORY_WRAPPER

#include "PreProcUtils.hpp"
#include <functional>

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

#define DEFINE_VK_MEMORY_WRAPPER(Type, Name, DestroyFunc)                      \
  class Name##Wrapper : public MemoryWrapper<Type> {                           \
     public:                                                                   \
    Name##Wrapper(                                                             \
        VkDevice dev                            = VK_NULL_HANDLE,              \
        Type object                             = VK_NULL_HANDLE,              \
        const VkAllocationCallbacks *pAllocator = nullptr                      \
    )                                                                          \
        : MemoryWrapper<Type>(                                                 \
              object,                                                          \
              std::function([this, dev, pAllocator]() {                        \
                if (this->get() != VK_NULL_HANDLE && dev != VK_NULL_HANDLE) {  \
                  DestroyFunc(dev, this->get(), pAllocator);                   \
                }                                                              \
              })                                                               \
          )                                                                    \
    {                                                                          \
    }                                                                          \
  };

#endif // MEMORY_WRAPPER
