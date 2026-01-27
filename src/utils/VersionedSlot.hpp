#ifndef VERSIONNED_SLOT_HPP
#define VERSIONNED_SLOT_HPP

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <utility>

#ifdef __linux__
  #include <linux/futex.h>
  #include <sys/syscall.h>
  #include <unistd.h>

inline int futex_wait(void *addr, uint32_t expected)
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

inline int futex_wake(void *addr, int num_wake)
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
#else
template <typename T>
inline int futex_wait(void *addr, uint32_t expected)
{
  while (static_cast<std::atomic<T> *>(addr)->load(std::memory_order_relaxed) ==
         static_cast<T>(expected)) {
  #ifdef __x86_64__
    __builtin_ia32_pause();
  #endif
  }
  return 0;
}

inline int futex_wake(void *addr, int num_wake)
{
  (void)addr;
  (void)num_wake;
  return 0;
}
#endif

/**
 * @brief Concept for valid versioned slot storage types
 */
template <typename T>
concept UIntWord = std::is_unsigned_v<T> &&
                   (sizeof(T) == 1 || sizeof(T) == 2 ||
                    sizeof(T) == 4 || sizeof(T) == 8);

/**
 * @brief Versioned resource slot with lock state
 *
 * Layout: [version:N-2][state:2]
 * - Upper N-2 bits: Version counter
 * - Lower 2 bits: State (FREE/UNLOCKED/LOCKED/CONTESTED)
 *
 * Size-based version limits:
 * - uint8_t:  6-bit version (64 max)
 * - uint16_t: 14-bit version (16,384 max)
 * - uint32_t: 30-bit version (1,073,741,824 max)
 * - uint64_t: 62-bit version (4,611,686,018,427,387,904 max)
 *
 * @tparam UWord Storage type (uint8_t, uint16_t, uint32_t, uint64_t)
 */
template <UIntWord UWord = uint16_t>
class VersionedSlot {
   public:
  // State values (2 bits)
  static constexpr UWord FREE      = 0b00;
  static constexpr UWord UNLOCKED  = 0b01;
  static constexpr UWord LOCKED    = 0b10;
  static constexpr UWord CONTESTED = 0b11;

  // Masks
  static constexpr UWord STATE_MASK   = 0x03; // Lower 2 bits
  static constexpr UWord VERSION_MASK = static_cast<UWord>(~static_cast<UWord>(0x03));

  // Version limits
  static constexpr int   VERSION_BITS = (sizeof(UWord) * 8) - 2;
  static constexpr UWord MAX_VERSION  = (static_cast<UWord>(1) << VERSION_BITS) - 1;

  // End-of-life threshold (95% of max)
  static constexpr UWord EOL_WARNING_THRESHOLD =
      static_cast<UWord>(MAX_VERSION * 0.95);

   private:
  std::atomic<UWord> word_;

   public:
  VersionedSlot()
      : word_(pack(0, FREE))
  {
  }

  // Accessors
  UWord version() const { return word_.load(std::memory_order_relaxed) >> 2; }

  UWord state() const
  {
    return word_.load(std::memory_order_relaxed) & STATE_MASK;
  }

  UWord load() const { return word_.load(std::memory_order_relaxed); }

  static constexpr UWord pack(UWord version, UWord state)
  {
    return ((version & MAX_VERSION) << 2) | (state & 0b11);
  }

  static constexpr UWord getVersion(UWord word) { return word >> 2; }

  static constexpr UWord getState(UWord word) { return word & STATE_MASK; }

  /**
   * @brief Check if slot is at end-of-life (version maxed out)
   */
  bool isEndOfLife() const { return version() >= MAX_VERSION; }

  /**
   * @brief Check if slot is approaching end-of-life
   * @return true if > 95% of versions used
   */
  bool isNearEndOfLife() const { return version() >= EOL_WARNING_THRESHOLD; }

  /**
   * @brief Get remaining versions before end-of-life
   */
  UWord remainingVersions() const
  {
    UWord ver = version();
    return ver < MAX_VERSION ? MAX_VERSION - ver : 0;
  }

  /**
   * @brief Allocate slot: FREE -> UNLOCKED (version unchanged)
   * @return {success, version, nearEOL}
   */
  struct AllocResult {
    bool  success;
    UWord version;
    bool  nearEndOfLife;
  };

  AllocResult tryAllocate()
  {
    UWord current = word_.load(std::memory_order_relaxed);

    // Check not at end-of-life
    if (getVersion(current) >= MAX_VERSION) {
      return {false, 0, true}; // Slot permanently retired
    }

    if (getState(current) != FREE) {
      return {false, 0, false};
    }

    // FREE -> UNLOCKED (keep version)
    UWord desired = (current & VERSION_MASK) | UNLOCKED;

    if (word_.compare_exchange_strong(
            current,
            desired,
            std::memory_order_acquire,
            std::memory_order_relaxed
        )) {
      UWord ver     = getVersion(desired);
      bool  nearEOL = ver >= EOL_WARNING_THRESHOLD;
      return {true, ver, nearEOL};
    }

    return {false, 0, false};
  }

  /**
   * @brief Free slot: UNLOCKED -> FREE (increment version)
   * @param expected_version Version to validate
   * @return true if freed successfully
   */
  bool free(UWord expected_version)
  {
    UWord current = word_.load(std::memory_order_relaxed);

    // Check version and state
    if (getVersion(current) != expected_version ||
        getState(current) != UNLOCKED) {
      return false;
    }

    // Increment version (or cap at MAX_VERSION for end-of-life)
    UWord new_version = expected_version + 1;
    if (new_version > MAX_VERSION) {
      new_version = MAX_VERSION; // Permanent end-of-life
    }

    UWord desired = pack(new_version, FREE);

    if (word_.compare_exchange_strong(
            current,
            desired,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {
      // Wake all waiters (version changed)
      futex_wake(&word_, INT32_MAX);
      return true;
    }

    return false;
  }

  /**
   * @brief Lock: UNLOCKED -> LOCKED (blocks until acquired)
   * @param expected_version Version to validate
   * @return true if locked, false if version mismatch
   */
  bool lock(UWord expected_version)
  {
    while (true) {
      UWord current = word_.load(std::memory_order_relaxed);

      // Validate version
      if (getVersion(current) != expected_version) {
        return false; // Stale version or slot freed
      }

      UWord st = getState(current);

      // Try UNLOCKED -> LOCKED
      if (st == UNLOCKED) {
        UWord desired = (current & VERSION_MASK) | LOCKED;

        if (word_.compare_exchange_weak(
                current,
                desired,
                std::memory_order_acquire,
                std::memory_order_relaxed
            )) {
          return true; // Acquired!
        }
        continue;
      }

      // Mark contested: LOCKED -> CONTESTED
      if (st == LOCKED) {
        UWord desired = (current & VERSION_MASK) | CONTESTED;
        word_.compare_exchange_weak(
            current,
            desired,
            std::memory_order_relaxed,
            std::memory_order_relaxed
        );
        current = word_.load(std::memory_order_relaxed);
      }

      // Wait for state or version change
      futex_wait(&word_, static_cast<uint32_t>(current));
    }
  }

  /**
   * @brief Try lock without blocking
   * @param expected_version Version to validate
   * @return true if locked, false otherwise
   */
  bool tryLock(UWord expected_version)
  {
    UWord current = word_.load(std::memory_order_relaxed);

    if (getVersion(current) != expected_version ||
        getState(current) != UNLOCKED) {
      return false;
    }

    UWord desired = (current & VERSION_MASK) | LOCKED;

    return word_.compare_exchange_strong(
        current,
        desired,
        std::memory_order_acquire,
        std::memory_order_relaxed
    );
  }

  /**
   * @brief Unlock: LOCKED/CONTESTED -> UNLOCKED
   * @param expected_version Version to validate
   * @return true if unlocked, false if version mismatch
   */
  bool unlock(UWord expected_version)
  {
    while (true) {
      UWord current = word_.load(std::memory_order_relaxed);

      // Validate version
      if (getVersion(current) != expected_version) {
        return false;
      }

      UWord st = getState(current);

      if (st == LOCKED || st == CONTESTED) {
        UWord desired = (current & VERSION_MASK) | UNLOCKED;

        if (word_.compare_exchange_strong(
                current,
                desired,
                std::memory_order_release,
                std::memory_order_relaxed
            )) {
          // Wake waiters if contested
          if (st == CONTESTED) {
            futex_wake(&word_, 1);
          }
          return true;
        }
        continue;
      }

      return false; // Not locked
    }
  }

  /**
   * @brief Check if handle is valid for given version
   */
  bool isValid(UWord expected_version) const
  {
    UWord current = word_.load(std::memory_order_relaxed);
    return getVersion(current) == expected_version &&
           getState(current) != FREE;
  }

  /**
   * @brief Get diagnostic info about this slot
   */
  struct DiagInfo {
    UWord version;
    UWord state;
    UWord remainingVersions;
    bool  isEndOfLife;
    bool  isNearEndOfLife;
  };

  DiagInfo getDiagnostics() const
  {
    UWord ver = version();
    return DiagInfo{ver,
                    state(),
                    ver < MAX_VERSION ? MAX_VERSION - ver : 0,
                    ver >= MAX_VERSION,
                    ver >= EOL_WARNING_THRESHOLD};
  }
};

// Common type aliases
using VersionedSlot8  = VersionedSlot<uint8_t>;  // 6-bit version (64 max)
using VersionedSlot16 = VersionedSlot<uint16_t>; // 14-bit version (16K max)
using VersionedSlot32 = VersionedSlot<uint32_t>; // 30-bit version (1B max)
using VersionedSlot64 = VersionedSlot<uint64_t>; // 62-bit version (quintillions)

#endif
