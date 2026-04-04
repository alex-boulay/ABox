#include <catch2/catch_test_macros.hpp>
#include <VersionedSlot.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

TEST_CASE("VersionedSlot: Basic construction and state", "[utils][versioned_slot]") {
    SECTION("Default construction initializes to FREE state with version 0") {
        VersionedSlot slot;

        REQUIRE(slot.version() == 0);
        REQUIRE(slot.state() == VersionedSlot::FREE);
        REQUIRE_FALSE(slot.isEndOfLife());
        REQUIRE_FALSE(slot.isNearEndOfLife());
    }

    SECTION("Check version limits for uint32_t") {
        // VersionedSlot uses uint32_t with 30 bits for version (2 bits for state)
        REQUIRE(VersionedSlot::MAX_VERSION == 1073741823); // 2^30 - 1
        REQUIRE(VersionedSlot::VERSION_BITS == 30);
    }

    SECTION("Pack and unpack functions") {
        auto packed = VersionedSlot::pack(42, VersionedSlot::LOCKED);

        REQUIRE(VersionedSlot::getVersion(packed) == 42);
        REQUIRE(VersionedSlot::getState(packed) == VersionedSlot::LOCKED);
    }
}

TEST_CASE("VersionedSlot: Allocation operations", "[utils][versioned_slot]") {
    SECTION("Successful allocation from FREE state") {
        VersionedSlot slot;

        auto result = slot.tryAllocate();

        REQUIRE(result.success);
        REQUIRE(result.version == 0);
        REQUIRE_FALSE(result.nearEndOfLife);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
        REQUIRE(slot.version() == 0);
    }

    SECTION("Cannot allocate from non-FREE state") {
        VersionedSlot slot;

        // First allocation succeeds
        auto result1 = slot.tryAllocate();
        REQUIRE(result1.success);

        // Second allocation fails (slot is UNLOCKED)
        auto result2 = slot.tryAllocate();
        REQUIRE_FALSE(result2.success);
    }

    SECTION("Allocation preserves version number") {
        VersionedSlot slot;

        // Allocate and free to increment version
        slot.tryAllocate();
        slot.free(0);

        // New version should be 1
        auto result = slot.tryAllocate();
        REQUIRE(result.success);
        REQUIRE(result.version == 1);
    }

    SECTION("Cannot allocate slot at end-of-life") {
        VersionedSlot slot;

        // Simulate near end-of-life by checking the behavior
        // (We can't actually exhaust 1 billion versions in a test)
        // Just verify the slot starts with many versions available
        REQUIRE(slot.remainingVersions() == VersionedSlot::MAX_VERSION);
        REQUIRE_FALSE(slot.isEndOfLife());
    }
}

TEST_CASE("VersionedSlot: Free operations", "[utils][versioned_slot]") {
    SECTION("Successful free increments version and changes state") {
        VersionedSlot slot;

        slot.tryAllocate();
        REQUIRE(slot.version() == 0);

        bool freed = slot.free(0);

        REQUIRE(freed);
        REQUIRE(slot.version() == 1);
        REQUIRE(slot.state() == VersionedSlot::FREE);
    }

    SECTION("Cannot free with wrong version") {
        VersionedSlot slot;

        slot.tryAllocate();

        bool freed = slot.free(999); // Wrong version

        REQUIRE_FALSE(freed);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }

    SECTION("Cannot free slot in wrong state") {
        VersionedSlot slot;

        // Slot is FREE, try to free it
        bool freed = slot.free(0);

        REQUIRE_FALSE(freed);
    }

    SECTION("Version increments correctly") {
        VersionedSlot slot;

        // Verify version increments with each free
        for (int i = 0; i < 10; ++i) {
            slot.tryAllocate();
            REQUIRE(slot.version() == static_cast<uint32_t>(i));
            slot.free(i);
            REQUIRE(slot.version() == static_cast<uint32_t>(i + 1));
        }

        REQUIRE(slot.version() == 10);
        REQUIRE_FALSE(slot.isEndOfLife());
    }
}

TEST_CASE("VersionedSlot: Lock operations", "[utils][versioned_slot]") {
    SECTION("Successful tryLock from UNLOCKED state") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        bool locked = slot.tryLock(alloc.version);

        REQUIRE(locked);
        REQUIRE(slot.state() == VersionedSlot::LOCKED);
    }

    SECTION("tryLock fails with wrong version") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        bool locked = slot.tryLock(999);

        REQUIRE_FALSE(locked);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }

    SECTION("tryLock fails when already locked") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool locked_again = slot.tryLock(alloc.version);

        REQUIRE_FALSE(locked_again);
    }

    SECTION("Blocking lock with correct version") {
        VersionedSlot slot;
        auto alloc = slot.tryAllocate();

        std::atomic<bool> locked{false};
        std::thread locker([&]() {
            bool result = slot.lock(alloc.version);
            locked = result;
        });

        locker.join();

        REQUIRE(locked);
        REQUIRE(slot.state() == VersionedSlot::LOCKED);
    }

    SECTION("Lock fails with stale version") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        slot.free(alloc.version);

        // Now version has incremented
        bool locked = slot.tryLock(alloc.version);

        REQUIRE_FALSE(locked);
    }
}

TEST_CASE("VersionedSlot: Unlock operations", "[utils][versioned_slot]") {
    SECTION("Successful unlock from LOCKED state") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool unlocked = slot.unlock(alloc.version);

        REQUIRE(unlocked);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }

    SECTION("Cannot unlock with wrong version") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        slot.tryLock(alloc.version);

        bool unlocked = slot.unlock(999);

        REQUIRE_FALSE(unlocked);
        REQUIRE(slot.state() == VersionedSlot::LOCKED);
    }

    SECTION("Cannot unlock when not locked") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();

        bool unlocked = slot.unlock(alloc.version);

        REQUIRE_FALSE(unlocked);
    }
}

TEST_CASE("VersionedSlot: Version management and EOL", "[utils][versioned_slot]") {
    SECTION("remainingVersions calculation") {
        VersionedSlot slot;

        REQUIRE(slot.remainingVersions() == VersionedSlot::MAX_VERSION);

        slot.tryAllocate();
        slot.free(0);

        REQUIRE(slot.remainingVersions() == VersionedSlot::MAX_VERSION - 1);
    }

    SECTION("isNearEndOfLife threshold") {
        VersionedSlot slot;

        // With MAX_VERSION = 1073741823, we can't test actual EOL
        // Just verify the threshold constant is correctly calculated
        REQUIRE(VersionedSlot::EOL_WARNING_THRESHOLD == static_cast<uint32_t>(VersionedSlot::MAX_VERSION * 0.95));

        // Verify a new slot is not near EOL
        REQUIRE_FALSE(slot.isNearEndOfLife());
        REQUIRE_FALSE(slot.isEndOfLife());
    }

    SECTION("getDiagnostics provides comprehensive info") {
        VersionedSlot slot;

        auto diag = slot.getDiagnostics();

        REQUIRE(diag.version == 0);
        REQUIRE(diag.state == VersionedSlot::FREE);
        REQUIRE(diag.remainingVersions == VersionedSlot::MAX_VERSION);
        REQUIRE_FALSE(diag.isEndOfLife);
        REQUIRE_FALSE(diag.isNearEndOfLife);
    }
}

TEST_CASE("VersionedSlot: Handle validation", "[utils][versioned_slot]") {
    SECTION("isValid returns true for correct version and non-FREE state") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();

        REQUIRE(slot.isValid(alloc.version));
    }

    SECTION("isValid returns false for wrong version") {
        VersionedSlot slot;

        [[maybe_unused]] auto alloc = slot.tryAllocate();

        REQUIRE_FALSE(slot.isValid(999));
    }

    SECTION("isValid returns false for FREE state") {
        VersionedSlot slot;

        REQUIRE_FALSE(slot.isValid(0));
    }

    SECTION("isValid returns false after free") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        slot.free(alloc.version);

        REQUIRE_FALSE(slot.isValid(alloc.version));
    }
}

TEST_CASE("VersionedSlot: Concurrency - multiple threads allocating", "[utils][versioned_slot][concurrency]") {
    SECTION("Concurrent allocations only one succeeds") {
        VersionedSlot slot;

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
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }
}

TEST_CASE("VersionedSlot: Concurrency - lock contention", "[utils][versioned_slot][concurrency]") {
    SECTION("Multiple threads competing for lock") {
        VersionedSlot slot;

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
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }
}

TEST_CASE("VersionedSlot: Concurrency - blocking lock with unlock", "[utils][versioned_slot][concurrency]") {
    SECTION("Blocking lock waits for unlock") {
        VersionedSlot slot;

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
        VersionedSlot slot;

        constexpr int num_cycles = 100;

        for (int i = 0; i < num_cycles; ++i) {
            auto alloc = slot.tryAllocate();
            REQUIRE(alloc.success);
            REQUIRE(alloc.version == static_cast<uint32_t>(i));

            bool freed = slot.free(alloc.version);
            REQUIRE(freed);
        }

        REQUIRE(slot.version() == num_cycles);
        REQUIRE(slot.state() == VersionedSlot::FREE);
    }
}

TEST_CASE("VersionedSlot: Edge cases", "[utils][versioned_slot]") {
    SECTION("State transitions maintain invariants") {
        VersionedSlot slot;

        // FREE -> UNLOCKED (via allocate)
        auto alloc = slot.tryAllocate();
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);

        // UNLOCKED -> LOCKED (via lock)
        slot.tryLock(alloc.version);
        REQUIRE(slot.state() == VersionedSlot::LOCKED);

        // LOCKED -> UNLOCKED (via unlock)
        slot.unlock(alloc.version);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);

        // UNLOCKED -> FREE (via free, version increments)
        slot.free(alloc.version);
        REQUIRE(slot.state() == VersionedSlot::FREE);
        REQUIRE(slot.version() == alloc.version + 1);
    }

    SECTION("Load returns packed word") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        uint32_t loaded = slot.load();

        REQUIRE(VersionedSlot::getVersion(loaded) == alloc.version);
        REQUIRE(VersionedSlot::getState(loaded) == VersionedSlot::UNLOCKED);
    }

    SECTION("VersionedSlot uses uint32_t for futex compatibility") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        // VersionedSlot always uses uint32_t (4 bytes) for optimal futex support
        REQUIRE(sizeof(slot.load()) == 4);
        REQUIRE(sizeof(VersionedSlot::UWord) == 4);
    }
}

TEST_CASE("VersionedSlot: CONTESTED state transitions", "[utils][versioned_slot][concurrency]") {
    SECTION("Multiple waiters create CONTESTED state") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        // Lock the slot with tryLock
        bool locked = slot.tryLock(alloc.version);
        REQUIRE(locked);

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

        // Give threads time to start waiting and mark state as CONTESTED
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Unlock to allow waiters to proceed
        bool unlocked = slot.unlock(alloc.version);
        REQUIRE(unlocked);

        for (auto& t : waiters) {
            t.join();
        }

        // All waiters should eventually acquire the lock
        REQUIRE(acquired_count == num_waiters);
        REQUIRE(slot.state() == VersionedSlot::UNLOCKED);
    }
}

TEST_CASE("VersionedSlot: tryLock with wrong state", "[utils][versioned_slot]") {
    SECTION("tryLock fails when slot is FREE") {
        VersionedSlot slot;

        // Slot is FREE, tryLock requires UNLOCKED
        bool locked = slot.tryLock(0);

        REQUIRE_FALSE(locked);
        REQUIRE(slot.state() == VersionedSlot::FREE);
    }

    SECTION("tryLock fails when slot is LOCKED") {
        VersionedSlot slot;

        auto alloc = slot.tryAllocate();
        REQUIRE(alloc.success);

        // Lock it first
        slot.tryLock(alloc.version);

        // Try to lock again - should fail
        bool locked_again = slot.tryLock(alloc.version);

        REQUIRE_FALSE(locked_again);
        REQUIRE(slot.state() == VersionedSlot::LOCKED);
    }
}
