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

  T      get() const { return container; }
  inline operator T() { return container; }
  ~MemoryWrapper() { destroy(); }
};
