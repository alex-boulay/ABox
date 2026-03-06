#include "platform/futex.hpp"

#include <windows.h>
#include <cstdint>

namespace abox::platform {

int futex_wait(void *addr, uint32_t expected)
{
  // WaitOnAddress blocks until *addr != expected
  WaitOnAddress(addr, &expected, sizeof(uint32_t), INFINITE);
  return 0;
}

int futex_wake(void *addr, int num_wake)
{
  if (num_wake == 1) {
    WakeByAddressSingle(addr);
  } else {
    // Wake all waiters
    WakeByAddressAll(addr);
  }
  return 0;
}

} // namespace abox::platform
