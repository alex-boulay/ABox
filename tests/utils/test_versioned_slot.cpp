#include <catch2/catch_test_macros.hpp>
#include <VersionedSlot.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

TEST_CASE("VersionedSlot: Basic construction and state", "[utils][versioned_slot]") {
    SECTION("Default construction initializes to FREE state with version 0") {
        VersionedSlot16 slot;

        REQUIRE(slot.version() == 0);
        REQUIRE(slot.state() == VersionedSlot16::FREE);
        REQUIRE_FALSE(slot.isEndOfLife());
        REQUIRE_FALSE(slot.isNearEndOfLife());
    }

    SECTION("Check type-specific version limits") {
        REQUIRE(VersionedSlot8::MAX_VERSION == 63);          // 2^6 - 1
        REQUIRE(VersionedSlot16::MAX_VERSION == 16383);      // 2^14 - 1
        REQUIRE(VersionedSlot32::MAX_VERSION == 1073741823); // 2^30 - 1

        REQUIRE(VersionedSlot8::VERSION_BITS == 6);
        REQUIRE(VersionedSlot16::VERSION_BITS == 14);
        REQUIRE(VersionedSlot32::VERSION_BITS == 30);
        REQUIRE(VersionedSlot64::VERSION_BITS == 62);
    }

    SECTION("Pack and unpack functions") {
        auto packed = VersionedSlot16::pack(42, VersionedSlot16::LOCKED);

        REQUIRE(VersionedSlot16::getVersion(packed) == 42);
        REQUIRE(VersionedSlot16::getState(packed) == VersionedSlot16::LOCKED);
    }
}

TEST_CASE("VersionedSlot: Allocation operations", "[utils][versioned_slot]") {
    SECTION("Successful allocation from FREE state") {
        VersionedSlot16 slot;

        auto result = slot.tryAllocate();

        REQUIRE(result.success);
        REQUIRE(result.version == 0);
        REQUIRE_FALSE(result.nearEndOfLife);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
        REQUIRE(slot.version() == 0);
    }

    SECTION("Cannot allocate from non-FREE state") {
        VersionedSlot16 slot;

        // First allocation succeeds
        auto result1 = slot.tryAllocate();
        REQUIRE(result1.success);

        // Second allocation fails (slot is UNLOCKED)
        auto result2 = slot.tryAllocate();
        REQUIRE_FALSE(result2.success);
    }

    SECTION("Allocation preserves version number") {
        VersionedSlot16 slot;

        // Allocate and free to increment version
        slot.tryAllocate();
        slot.free(0);

        // New version should be 1
        auto result = slot.tryAllocate();
        REQUIRE(result.success);
        REQUIRE(result.version == 1);
    }

    SECTION("Cannot allocate slot at end-of-life") {
        VersionedSlot8 slot;

        // Exhaust all versions (max is 63 for uint8_t)
        for (int i = 0; i < 64; ++i) {
            auto alloc_result = slot.tryAllocate();
            if (!alloc_result.success) {
                REQUIRE(alloc_result.nearEndOfLife);
                break;
            }
            slot.free(i);
        }

        // Should not be able to allocate anymore
        auto result = slot.tryAllocate();
        REQUIRE_FALSE(result.success);
        REQUIRE(result.nearEndOfLife);
    }
}

TEST_CASE("VersionedSlot: Free operations", "[utils][versioned_slot]") {
    SECTION("Successful free increments version and changes state") {
        VersionedSlot16 slot;

        slot.tryAllocate();
        REQUIRE(slot.version() == 0);

        bool freed = slot.free(0);

        REQUIRE(freed);
        REQUIRE(slot.version() == 1);
        REQUIRE(slot.state() == VersionedSlot16::FREE);
    }

    SECTION("Cannot free with wrong version") {
        VersionedSlot16 slot;

        slot.tryAllocate();

        bool freed = slot.free(999); // Wrong version

        REQUIRE_FALSE(freed);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }

    SECTION("Cannot free slot in wrong state") {
        VersionedSlot16 slot;

        // Slot is FREE, try to free it
        bool freed = slot.free(0);

        REQUIRE_FALSE(freed);
    }

    SECTION("Version caps at MAX_VERSION at end-of-life") {
        VersionedSlot8 slot;

        // Get to MAX_VERSION - 1
        for (int i = 0; i < 63; ++i) {
            slot.tryAllocate();
            slot.free(i);
        }

        REQUIRE(slot.version() == 63);
        REQUIRE(slot.isEndOfLife());

        // Try to allocate one more time - should fail
        auto result = slot.tryAllocate();
        REQUIRE_FALSE(result.success);
    }
}

TEST_CASE("VersionedSlot: Lock operations", "[utils][versioned_slot]") {
    SECTION("Successful tryLock from UNLOCKED state") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        bool locked = slot.tryLock(alloc.version);

        REQUIRE(locked);
        REQUIRE(slot.state() == VersionedSlot16::LOCKED);
    }

    SECTION("tryLock fails with wrong version") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        bool locked = slot.tryLock(999);

        REQUIRE_FALSE(locked);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }

    SECTION("tryLock fails when already locked") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool locked_again = slot.tryLock(alloc.version);

        REQUIRE_FALSE(locked_again);
    }

    SECTION("Blocking lock with correct version") {
        VersionedSlot16 slot;
        auto alloc = slot.tryAllocate();

        std::atomic<bool> locked{false};
        std::thread locker([&]() {
            bool result = slot.lock(alloc.version);
            locked = result;
        });

        locker.join();

        REQUIRE(locked);
        REQUIRE(slot.state() == VersionedSlot16::LOCKED);
    }

    SECTION("Lock fails with stale version") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        slot.free(alloc.version);

        // Now version has incremented
        bool locked = slot.tryLock(alloc.version);

        REQUIRE_FALSE(locked);
    }
}

TEST_CASE("VersionedSlot: Unlock operations", "[utils][versioned_slot]") {
    SECTION("Successful unlock from LOCKED state") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool unlocked = slot.unlock(alloc.version);

        REQUIRE(unlocked);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }

    SECTION("Cannot unlock with wrong version") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool unlocked = slot.unlock(999);

        REQUIRE_FALSE(unlocked);
        REQUIRE(slot.state() == VersionedSlot16::LOCKED);
    }

    SECTION("Cannot unlock when not locked") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();

        bool unlocked = slot.unlock(alloc.version);

        REQUIRE_FALSE(unlocked);
    }
}

TEST_CASE("VersionedSlot: Version management and EOL", "[utils][versioned_slot]") {
    SECTION("remainingVersions calculation") {
        VersionedSlot8 slot;

        REQUIRE(slot.remainingVersions() == 63);

        slot.tryAllocate();
        slot.free(0);

        REQUIRE(slot.remainingVersions() == 62);
    }

    SECTION("isNearEndOfLife threshold") {
        VersionedSlot8 slot;

        // Get to 95% of max (63 * 0.95 = 59.85, so ~60)
        for (int i = 0; i < 60; ++i) {
            slot.tryAllocate();
            slot.free(i);
        }

        REQUIRE(slot.isNearEndOfLife());
        REQUIRE_FALSE(slot.isEndOfLife());
    }

    SECTION("Allocation reports nearEndOfLife") {
        VersionedSlot8 slot;

        // Get close to EOL
        for (int i = 0; i < 60; ++i) {
            slot.tryAllocate();
            slot.free(i);
        }

        auto result = slot.tryAllocate();
        REQUIRE(result.success);
        REQUIRE(result.nearEndOfLife);
    }

    SECTION("getDiagnostics provides comprehensive info") {
        VersionedSlot8 slot;

        auto diag = slot.getDiagnostics();

        REQUIRE(diag.version == 0);
        REQUIRE(diag.state == VersionedSlot8::FREE);
        REQUIRE(diag.remainingVersions == 63);
        REQUIRE_FALSE(diag.isEndOfLife);
        REQUIRE_FALSE(diag.isNearEndOfLife);
    }
}

TEST_CASE("VersionedSlot: Handle validation", "[utils][versioned_slot]") {
    SECTION("isValid returns true for correct version and non-FREE state") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();

        REQUIRE(slot.isValid(alloc.version));
    }

    SECTION("isValid returns false for wrong version") {
        VersionedSlot16 slot;

        [[maybe_unused]] auto alloc = slot.tryAllocate();

        REQUIRE_FALSE(slot.isValid(999));
    }

    SECTION("isValid returns false for FREE state") {
        VersionedSlot16 slot;

        REQUIRE_FALSE(slot.isValid(0));
    }

    SECTION("isValid returns false after free") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        slot.free(alloc.version);

        REQUIRE_FALSE(slot.isValid(alloc.version));
    }
}

TEST_CASE("VersionedSlot: Concurrency - multiple threads allocating", "[utils][versioned_slot][concurrency]") {
    SECTION("Concurrent allocations only one succeeds") {
        VersionedSlot16 slot;

        constexpr int num_threads = 10;
        std::atomic<int> success_count{0};
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                auto result = slot.tryAllocate();
                if (result.success) {
                    success_count++;
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        REQUIRE(success_count == 1);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }
}

TEST_CASE("VersionedSlot: Concurrency - lock contention", "[utils][versioned_slot][concurrency]") {
    SECTION("Multiple threads competing for lock") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        constexpr int num_threads = 5;
        std::atomic<int> lock_success_count{0};
        std::atomic<int> operations_completed{0};
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&, version = alloc.version]() {
                if (slot.tryLock(version)) {
                    lock_success_count++;

                    // Hold lock briefly
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    slot.unlock(version);
                    operations_completed++;
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        // At least one thread should have succeeded
        REQUIRE(lock_success_count >= 1);
        REQUIRE(operations_completed >= 1);

        // Final state should be UNLOCKED
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }
}

TEST_CASE("VersionedSlot: Concurrency - blocking lock with unlock", "[utils][versioned_slot][concurrency]") {
    SECTION("Blocking lock waits for unlock") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        // First thread locks
        slot.tryLock(alloc.version);

        std::atomic<bool> second_thread_acquired{false};
        std::atomic<bool> unlocked{false};

        // Second thread tries to lock (will block)
        std::thread waiter([&]() {
            bool result = slot.lock(alloc.version);
            second_thread_acquired = result;
        });

        // Give waiter time to start waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Unlock from main thread
        unlocked = slot.unlock(alloc.version);

        waiter.join();

        REQUIRE(unlocked);
        REQUIRE(second_thread_acquired);
    }
}

TEST_CASE("VersionedSlot: Concurrency - allocation cycle", "[utils][versioned_slot][concurrency]") {
    SECTION("Allocate-free-allocate cycle maintains consistency") {
        VersionedSlot16 slot;

        constexpr int num_cycles = 100;

        for (int i = 0; i < num_cycles; ++i) {
            auto alloc = slot.tryAllocate();
            REQUIRE(alloc.success);
            REQUIRE(alloc.version == static_cast<uint16_t>(i));

            bool freed = slot.free(alloc.version);
            REQUIRE(freed);
        }

        REQUIRE(slot.version() == num_cycles);
        REQUIRE(slot.state() == VersionedSlot16::FREE);
    }
}

TEST_CASE("VersionedSlot: Edge cases", "[utils][versioned_slot]") {
    SECTION("State transitions maintain invariants") {
        VersionedSlot16 slot;

        // FREE -> UNLOCKED (via allocate)
        auto alloc = slot.tryAllocate();
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);

        // UNLOCKED -> LOCKED (via lock)
        slot.tryLock(alloc.version);
        REQUIRE(slot.state() == VersionedSlot16::LOCKED);

        // LOCKED -> UNLOCKED (via unlock)
        slot.unlock(alloc.version);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);

        // UNLOCKED -> FREE (via free, version increments)
        slot.free(alloc.version);
        REQUIRE(slot.state() == VersionedSlot16::FREE);
        REQUIRE(slot.version() == alloc.version + 1);
    }

    SECTION("Load returns packed word") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        uint16_t loaded = slot.load();

        REQUIRE(VersionedSlot16::getVersion(loaded) == alloc.version);
        REQUIRE(VersionedSlot16::getState(loaded) == VersionedSlot16::UNLOCKED);
    }

    SECTION("Different slot sizes work independently") {
        VersionedSlot8 slot8;
        VersionedSlot16 slot16;
        VersionedSlot32 slot32;

        auto alloc8 = slot8.tryAllocate();
        auto alloc16 = slot16.tryAllocate();
        auto alloc32 = slot32.tryAllocate();

        REQUIRE(alloc8.success);
        REQUIRE(alloc16.success);
        REQUIRE(alloc32.success);

        REQUIRE(sizeof(slot8.load()) == 1);
        REQUIRE(sizeof(slot16.load()) == 2);
        REQUIRE(sizeof(slot32.load()) == 4);
    }
}

TEST_CASE("VersionedSlot: CONTESTED state transitions", "[utils][versioned_slot][concurrency]") {
    SECTION("Multiple waiters create CONTESTED state") {
        VersionedSlot16 slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        // Lock the slot
        slot.lock(alloc.version);

        constexpr int num_waiters = 3;
        std::vector<std::thread> waiters;
        std::atomic<int> acquired_count{0};

        // Create multiple threads that will wait for lock
        for (int i = 0; i < num_waiters; ++i) {
            waiters.emplace_back([&, version = alloc.version]() {
                if (slot.lock(version)) {
                    acquired_count++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    slot.unlock(version);
                }
            });
        }

        // Give threads time to start waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Unlock to allow waiters to proceed
        slot.unlock(alloc.version);

        for (auto& t : waiters) {
            t.join();
        }

        // All waiters should eventually acquire the lock
        REQUIRE(acquired_count == num_waiters);
        REQUIRE(slot.state() == VersionedSlot16::UNLOCKED);
    }
}
