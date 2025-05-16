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
