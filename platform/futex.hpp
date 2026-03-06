#ifndef ABOX_PLATFORM_FUTEX_HPP
#define ABOX_PLATFORM_FUTEX_HPP

#include <cstdint>

namespace abox::platform {

/**
 * @brief Wait on a futex (blocks until value changes or spurious wakeup)
 * @param addr Address of the atomic variable to wait on
 * @param expected Expected value - will wait if *addr == expected
 * @return 0 on success, -1 on error (check errno on Linux)
 */
int futex_wait(void *addr, uint32_t expected);

/**
 * @brief Wake threads waiting on a futex
 * @param addr Address of the atomic variable
 * @param num_wake Number of threads to wake (INT32_MAX for all)
 * @return Number of threads woken, or 0
 */
int futex_wake(void *addr, int num_wake);

} // namespace abox::platform

#endif // ABOX_PLATFORM_FUTEX_HPP
