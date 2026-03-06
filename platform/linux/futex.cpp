#include "platform/futex.hpp"

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace abox::platform {

int futex_wait(void *addr, uint32_t expected)
{
  return syscall(
      SYS_futex,
      addr,
      FUTEX_WAIT_PRIVATE,
      expected,
      nullptr,
      nullptr,
      0
  );
}

int futex_wake(void *addr, int num_wake)
{
  return syscall(
      SYS_futex,
      addr,
      FUTEX_WAKE_PRIVATE,
      num_wake,
      nullptr,
      nullptr,
      0
  );
}

} // namespace abox::platform
