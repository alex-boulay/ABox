#include "platform/futex.hpp"

#include <atomic>
#include <cstdint>

namespace abox::platform {

int futex_wait(void *addr, uint32_t expected)
{
  // Spin-wait fallback for platforms without native futex support
  auto *atomic_addr = static_cast<std::atomic<uint32_t> *>(addr);

  while (atomic_addr->load(std::memory_order_relaxed) == expected) {
#ifdef __x86_64__
    __builtin_ia32_pause(); // CPU hint for spin-wait loops
#elif defined(__aarch64__)
    __asm__ __volatile__("yield"); // ARM yield instruction
#endif
  }

  return 0;
}

int futex_wake(void *addr, int num_wake)
{
  // No-op in fallback (spin-waiters check atomically)
  (void)addr;
  (void)num_wake;
  return 0;
}

} // namespace abox::platform
