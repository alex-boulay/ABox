#include <catch2/catch_test_macros.hpp>
#include <FetchList.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

// Simple test struct
struct TestObject {
    int value;
    std::string name;

    TestObject(int v, std::string n)
        : value(v)
        , name(std::move(n))
    {
    }
};

TEST_CASE("FetchList: Basic construction and properties", "[utils][fetch_list]") {
    SECTION("Default construction creates empty list") {
        FetchList<int> list;

        REQUIRE(list.size() == 0);
        REQUIRE(list.capacity() == 0);
        REQUIRE(list.empty());
    }

    SECTION("Custom multiplier construction") {
        FetchList<int> list(4);

        REQUIRE(list.size() == 0);
        REQUIRE(list.empty());
    }

    SECTION("Cache-aligned multiplier calculation") {
        constexpr size_t mult = cache_aligned_multplier<int>();

        // Should return a reasonable value between 1 and 16
        REQUIRE(mult >= 1);
        REQUIRE(mult <= 16);
    }
}

TEST_CASE("FetchList: Handle structure", "[utils][fetch_list]") {
    SECTION("Default handle is invalid") {
        FetchList<int>::Handle handle;

        REQUIRE_FALSE(handle.isValid());
        REQUIRE(handle.index == static_cast<size_t>(-1));
        REQUIRE(handle.version == 0);
    }

    SECTION("Constructed handle with valid index") {
        FetchList<int>::Handle handle(42, 7);

        REQUIRE(handle.isValid());
        REQUIRE(handle.index == 42);
        REQUIRE(handle.version == 7);
    }

    SECTION("Handle equality comparison") {
        FetchList<int>::Handle h1(10, 5);
        FetchList<int>::Handle h2(10, 5);
        FetchList<int>::Handle h3(10, 6);
        FetchList<int>::Handle h4(11, 5);

        REQUIRE(h1 == h2);
        REQUIRE(h1 != h3);
        REQUIRE(h1 != h4);
    }
}

TEST_CASE("FetchList: Emplace and get operations", "[utils][fetch_list]") {
    SECTION("Emplace single element") {
        FetchList<int> list;

        auto handle = list.emplace(42);

        REQUIRE(handle.isValid());
        REQUIRE(list.size() == 1);
        REQUIRE_FALSE(list.empty());
        REQUIRE(list.capacity() > 0);
    }

    SECTION("Get element by handle") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        int* ptr = list.get(handle);

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == 42);
    }

    SECTION("Const get") {
        FetchList<int> list;
        auto handle = list.emplace(99);

        const FetchList<int>& const_list = list;
        const int* ptr = const_list.get(handle);

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == 99);
    }

    SECTION("Emplace complex object") {
        FetchList<TestObject> list;

        auto handle = list.emplace(123, "test");
        TestObject* obj = list.get(handle);

        REQUIRE(obj != nullptr);
        REQUIRE(obj->value == 123);
        REQUIRE(obj->name == "test");
    }

    SECTION("Multiple emplaces") {
        FetchList<int> list;

        auto h1 = list.emplace(1);
        auto h2 = list.emplace(2);
        auto h3 = list.emplace(3);

        REQUIRE(list.size() == 3);
        REQUIRE(*list.get(h1) == 1);
        REQUIRE(*list.get(h2) == 2);
        REQUIRE(*list.get(h3) == 3);
    }
}

TEST_CASE("FetchList: Erase operations", "[utils][fetch_list]") {
    SECTION("Erase single element") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        REQUIRE(list.size() == 1);

        bool erased = list.erase(handle);

        REQUIRE(erased);
        REQUIRE(list.size() == 0);
        REQUIRE(list.empty());
    }

    SECTION("Get after erase returns nullptr") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        list.erase(handle);

        int* ptr = list.get(handle);

        REQUIRE(ptr == nullptr);
    }

    SECTION("Cannot erase with invalid handle") {
        FetchList<int> list;

        FetchList<int>::Handle invalid_handle;
        bool erased = list.erase(invalid_handle);

        REQUIRE_FALSE(erased);
    }

    SECTION("Cannot erase with stale handle (version mismatch)") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        list.erase(handle);

        // Try to erase again with same handle (version is now stale)
        bool erased_again = list.erase(handle);

        REQUIRE_FALSE(erased_again);
    }

    SECTION("Erase calls destructor") {
        static std::atomic<int> destructor_count{0};

        struct DestructorCounter {
            DestructorCounter() { }
            ~DestructorCounter() { destructor_count++; }
        };

        destructor_count = 0;

        {
            FetchList<DestructorCounter> list;
            auto h1 = list.emplace();
            auto h2 = list.emplace();

            list.erase(h1);
            REQUIRE(destructor_count == 1);

            list.erase(h2);
            REQUIRE(destructor_count == 2);
        }

        // Verify destructor count (may include list destruction cleanup)
        REQUIRE(destructor_count >= 2);
    }
}

TEST_CASE("FetchList: Handle validation", "[utils][fetch_list]") {
    SECTION("isValid returns true for valid handle") {
        FetchList<int> list;

        auto handle = list.emplace(42);

        REQUIRE(list.isValid(handle));
    }

    SECTION("isValid returns false for invalid handle") {
        FetchList<int> list;

        FetchList<int>::Handle invalid_handle;

        REQUIRE_FALSE(list.isValid(invalid_handle));
    }

    SECTION("isValid returns false after erase") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        list.erase(handle);

        REQUIRE_FALSE(list.isValid(handle));
    }

    SECTION("isValid returns false for out-of-bounds index") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        FetchList<int>::Handle oob_handle(999999, handle.version);

        REQUIRE_FALSE(list.isValid(oob_handle));
    }
}

TEST_CASE("FetchList: Growth and capacity", "[utils][fetch_list]") {
    SECTION("List grows automatically when full") {
        FetchList<int> list;

        std::vector<FetchList<int>::Handle> handles;

        // Add many elements to trigger growth
        for (int i = 0; i < 100; ++i) {
            handles.push_back(list.emplace(i));
        }

        REQUIRE(list.size() == 100);
        REQUIRE(list.capacity() >= 100);

        // Verify all elements are accessible
        for (size_t i = 0; i < handles.size(); ++i) {
            REQUIRE(*list.get(handles[i]) == static_cast<int>(i));
        }
    }

    SECTION("Capacity increases in blocks") {
        FetchList<int> list(2); // 8 * 2 = 16 elements per block

        for (int i = 0; i < 20; ++i) {
            list.emplace(i);
        }

        // Should have at least 2 blocks (32 capacity)
        REQUIRE(list.capacity() >= 32);
    }
}

TEST_CASE("FetchList: Slot reuse with versioning", "[utils][fetch_list]") {
    SECTION("Freed slots are reused") {
        FetchList<int> list;

        auto h1 = list.emplace(1);
        size_t first_index = h1.index;

        list.erase(h1);

        auto h2 = list.emplace(2);

        // Should reuse the same slot with different version
        REQUIRE(h2.index == first_index);
        REQUIRE(h2.version != h1.version);
    }

    SECTION("Version tracking prevents use-after-free") {
        FetchList<int> list;

        auto h1 = list.emplace(42);
        list.erase(h1);

        // Reuse the slot
        auto h2 = list.emplace(99);

        // Old handle should not work (version mismatch)
        REQUIRE_FALSE(list.isValid(h1));
        REQUIRE(list.get(h1) == nullptr);

        // New handle should work
        REQUIRE(list.isValid(h2));
        REQUIRE(*list.get(h2) == 99);
    }

    SECTION("ABA problem prevention") {
        FetchList<int> list;

        auto original = list.emplace(1);
        size_t index = original.index;
        auto version1 = original.version;

        list.erase(original);
        auto second = list.emplace(2);

        REQUIRE(second.index == index);
        REQUIRE(second.version != version1);

        list.erase(second);
        auto third = list.emplace(3);

        REQUIRE(third.index == index);
        REQUIRE(third.version != version1);
        REQUIRE(third.version != second.version);

        // Original handle should still be invalid
        REQUIRE_FALSE(list.isValid(original));
        REQUIRE(list.get(original) == nullptr);
    }
}

TEST_CASE("FetchList: Stable pointers", "[utils][fetch_list]") {
    SECTION("Pointers remain stable during growth") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        int* ptr1 = list.get(handle);

        // Trigger growth by adding many elements
        for (int i = 0; i < 100; ++i) {
            list.emplace(i);
        }

        int* ptr2 = list.get(handle);

        // Pointer should remain the same
        REQUIRE(ptr1 == ptr2);
        REQUIRE(*ptr2 == 42);
    }
}

TEST_CASE("FetchList: Thread safety requirements", "[utils][fetch_list]") {
    SECTION("FetchList requires external synchronization for concurrent access") {
        // FetchList is NOT thread-safe by itself
        // Users must provide external synchronization (e.g., mutex) for concurrent access
        //
        // This test documents the expected usage pattern for multi-threaded scenarios

        FetchList<int> list;
        std::mutex list_mutex;

        constexpr int num_threads = 10;
        constexpr int elements_per_thread = 100;
        std::vector<std::thread> threads;
        std::vector<std::vector<FetchList<int>::Handle>> all_handles(num_threads);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < elements_per_thread; ++i) {
                    std::lock_guard<std::mutex> lock(list_mutex);
                    auto handle = list.emplace(t * 1000 + i);
                    all_handles[t].push_back(handle);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Verify all elements were added
        REQUIRE(list.size() == num_threads * elements_per_thread);

        // Verify all handles are valid and point to correct values
        for (int t = 0; t < num_threads; ++t) {
            for (int i = 0; i < elements_per_thread; ++i) {
                auto handle = all_handles[t][i];
                REQUIRE(list.isValid(handle));

                int* ptr = list.get(handle);
                REQUIRE(ptr != nullptr);
                REQUIRE(*ptr == t * 1000 + i);
            }
        }
    }
}

TEST_CASE("FetchList: Concurrent emplace and erase with mutex", "[utils][fetch_list][concurrency]") {
    SECTION("Concurrent emplace and erase operations with synchronization") {
        FetchList<int> list;
        std::mutex list_mutex;

        std::atomic<bool> stop{false};
        std::atomic<int> operations{0};

        // Thread that emplaces elements
        std::thread adder([&]() {
            for (int i = 0; i < 200 && !stop; ++i) {
                {
                    std::lock_guard<std::mutex> lock(list_mutex);
                    list.emplace(i);
                }
                operations++;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });

        // Thread that removes elements
        std::thread remover([&]() {
            std::vector<FetchList<int>::Handle> to_remove;

            for (int i = 0; i < 100 && !stop; ++i) {
                {
                    std::lock_guard<std::mutex> lock(list_mutex);
                    auto handle = list.emplace(i + 10000);
                    to_remove.push_back(handle);

                    if (!to_remove.empty()) {
                        list.erase(to_remove.back());
                        to_remove.pop_back();
                    }
                }

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });

        adder.join();
        remover.join();

        stop = true;

        // List should be in a consistent state
        REQUIRE(operations > 0);
    }
}

TEST_CASE("FetchList: Destruction cleanup", "[utils][fetch_list]") {
    SECTION("Destructor destroys all elements") {
        static std::atomic<int> construction_count{0};
        static std::atomic<int> destruction_count{0};

        struct DestructorTracker {
            int id;

            DestructorTracker(int i) : id(i) {
                construction_count++;
            }

            ~DestructorTracker() {
                destruction_count++;
            }
        };

        construction_count = 0;
        destruction_count = 0;

        {
            FetchList<DestructorTracker> list;

            list.emplace(1);
            list.emplace(2);
            list.emplace(3);

            REQUIRE(construction_count == 3);
        }

        // All elements should be destroyed
        REQUIRE(destruction_count == 3);
    }

    SECTION("Destructor only destroys active elements") {
        static std::atomic<int> destruction_count2{0};

        struct DestructorTracker {
            ~DestructorTracker() {
                destruction_count2++;
            }
        };

        destruction_count2 = 0;

        {
            FetchList<DestructorTracker> list;

            [[maybe_unused]] auto h1 = list.emplace();
            auto h2 = list.emplace();
            [[maybe_unused]] auto h3 = list.emplace();

            list.erase(h2); // Destroys one element

            REQUIRE(destruction_count2 == 1);
        }

        // Should destroy the remaining 2 elements
        REQUIRE(destruction_count2 == 3);
    }
}

TEST_CASE("FetchList: Edge cases", "[utils][fetch_list]") {
    SECTION("Get with default handle returns nullptr") {
        FetchList<int> list;

        FetchList<int>::Handle default_handle;
        int* ptr = list.get(default_handle);

        REQUIRE(ptr == nullptr);
    }

    SECTION("Erase with out-of-bounds index returns false") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        FetchList<int>::Handle oob_handle(99999, handle.version);

        bool erased = list.erase(oob_handle);

        REQUIRE_FALSE(erased);
    }

    SECTION("Size tracking is accurate") {
        FetchList<int> list;

        REQUIRE(list.size() == 0);

        auto h1 = list.emplace(1);
        REQUIRE(list.size() == 1);

        auto h2 = list.emplace(2);
        REQUIRE(list.size() == 2);

        list.erase(h1);
        REQUIRE(list.size() == 1);

        list.erase(h2);
        REQUIRE(list.size() == 0);
    }

    SECTION("Empty check is accurate") {
        FetchList<int> list;

        REQUIRE(list.empty());

        auto handle = list.emplace(42);
        REQUIRE_FALSE(list.empty());

        list.erase(handle);
        REQUIRE(list.empty());
    }
}

TEST_CASE("FetchList: at() method", "[utils][fetch_list]") {
    SECTION("at() returns reference to element") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        int& value = list.at(handle);

        REQUIRE(value == 42);

        // Modify through reference
        value = 99;
        REQUIRE(*list.get(handle) == 99);
    }

    SECTION("at() throws on invalid handle") {
        FetchList<int> list;

        FetchList<int>::Handle invalid_handle;

        REQUIRE_THROWS_AS(list.at(invalid_handle), std::out_of_range);
    }

    SECTION("at() throws on stale handle") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        list.erase(handle);

        REQUIRE_THROWS_AS(list.at(handle), std::out_of_range);
    }

    SECTION("const at() works") {
        FetchList<int> list;
        auto handle = list.emplace(42);

        const FetchList<int>& const_list = list;
        const int& value = const_list.at(handle);

        REQUIRE(value == 42);
    }
}

TEST_CASE("FetchList: contains() method", "[utils][fetch_list]") {
    SECTION("contains() returns true for valid handle") {
        FetchList<int> list;

        auto handle = list.emplace(42);

        REQUIRE(list.contains(handle));
    }

    SECTION("contains() returns false for invalid handle") {
        FetchList<int> list;

        FetchList<int>::Handle invalid_handle;

        REQUIRE_FALSE(list.contains(invalid_handle));
    }

    SECTION("contains() returns false after erase") {
        FetchList<int> list;

        auto handle = list.emplace(42);
        list.erase(handle);

        REQUIRE_FALSE(list.contains(handle));
    }

    SECTION("contains() returns false for stale handle") {
        FetchList<int> list;

        auto h1 = list.emplace(1);
        list.erase(h1);
        auto h2 = list.emplace(2);  // Reuses slot

        REQUIRE_FALSE(list.contains(h1));
        REQUIRE(list.contains(h2));
    }
}

TEST_CASE("FetchList: getHandleByIndex() method", "[utils][fetch_list]") {
    SECTION("getHandleByIndex returns handle to nth element") {
        FetchList<int> list;

        auto h1 = list.emplace(10);
        auto h2 = list.emplace(20);
        auto h3 = list.emplace(30);

        auto handle0 = list.getHandleByIndex(0);
        auto handle1 = list.getHandleByIndex(1);
        auto handle2 = list.getHandleByIndex(2);

        REQUIRE(handle0 == h1);
        REQUIRE(handle1 == h2);
        REQUIRE(handle2 == h3);
    }

    SECTION("getHandleByIndex returns invalid handle for out of range") {
        FetchList<int> list;

        list.emplace(10);
        list.emplace(20);

        auto invalid = list.getHandleByIndex(5);

        REQUIRE_FALSE(invalid.isValid());
    }

    SECTION("getHandleByIndex skips erased elements") {
        FetchList<int> list;

        auto h1 = list.emplace(10);
        auto h2 = list.emplace(20);
        auto h3 = list.emplace(30);

        list.erase(h2);  // Erase middle element

        // Now we have 2 valid elements: h1 and h3
        auto handle0 = list.getHandleByIndex(0);
        auto handle1 = list.getHandleByIndex(1);

        REQUIRE(handle0 == h1);
        REQUIRE(handle1 == h3);
        REQUIRE(*list.get(handle0) == 10);
        REQUIRE(*list.get(handle1) == 30);
    }

    SECTION("getHandleByIndex works with sparse list") {
        FetchList<int> list;

        [[maybe_unused]] auto h1 = list.emplace(1);
        auto h2 = list.emplace(2);
        [[maybe_unused]] auto h3 = list.emplace(3);
        auto h4 = list.emplace(4);
        [[maybe_unused]] auto h5 = list.emplace(5);

        // Erase some elements
        list.erase(h2);
        list.erase(h4);

        // Valid elements: h1(1), h3(3), h5(5)
        REQUIRE(*list.get(list.getHandleByIndex(0)) == 1);
        REQUIRE(*list.get(list.getHandleByIndex(1)) == 3);
        REQUIRE(*list.get(list.getHandleByIndex(2)) == 5);
    }
}

TEST_CASE("FetchList: Iterator support", "[utils][fetch_list]") {
    SECTION("Range-based for loop over elements") {
        FetchList<int> list;

        list.emplace(1);
        list.emplace(2);
        list.emplace(3);

        int sum = 0;
        for (auto& value : list) {
            sum += value;
        }

        REQUIRE(sum == 6);
    }

    SECTION("Iterator skips erased elements") {
        FetchList<int> list;

        [[maybe_unused]] auto h1 = list.emplace(10);
        auto h2 = list.emplace(20);
        [[maybe_unused]] auto h3 = list.emplace(30);

        list.erase(h2);

        std::vector<int> values;
        for (const auto& value : list) {
            values.push_back(value);
        }

        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 30);
    }

    SECTION("Iterator works with empty list") {
        FetchList<int> list;

        int count = 0;
        for ([[maybe_unused]] const auto& value : list) {
            count++;
        }

        REQUIRE(count == 0);
    }

    SECTION("Const iterator works") {
        FetchList<int> list;

        list.emplace(1);
        list.emplace(2);

        const FetchList<int>& const_list = list;

        int sum = 0;
        for (const auto& value : const_list) {
            sum += value;
        }

        REQUIRE(sum == 3);
    }

    SECTION("Modify elements through iterator") {
        FetchList<int> list;

        list.emplace(1);
        list.emplace(2);
        list.emplace(3);

        // Multiply all by 2
        for (auto& value : list) {
            value *= 2;
        }

        std::vector<int> values;
        for (const auto& value : list) {
            values.push_back(value);
        }

        REQUIRE(values[0] == 2);
        REQUIRE(values[1] == 4);
        REQUIRE(values[2] == 6);
    }

    SECTION("Iterator with complex object") {
        FetchList<TestObject> list;

        list.emplace(1, "one");
        list.emplace(2, "two");
        list.emplace(3, "three");

        int value_sum = 0;
        for (const auto& obj : list) {
            value_sum += obj.value;
        }

        REQUIRE(value_sum == 6);
    }

    SECTION("begin() and end() work correctly") {
        FetchList<int> list;

        list.emplace(10);
        list.emplace(20);

        auto it = list.begin();
        REQUIRE(*it == 10);

        ++it;
        REQUIRE(*it == 20);

        ++it;
        REQUIRE(it == list.end());
    }

    SECTION("cbegin() and cend() work") {
        FetchList<int> list;

        list.emplace(5);
        list.emplace(10);

        int sum = 0;
        for (auto it = list.cbegin(); it != list.cend(); ++it) {
            sum += *it;
        }

        REQUIRE(sum == 15);
    }
}

TEST_CASE("FetchList: Additional at() edge cases", "[utils][fetch_list]") {
    SECTION("const at() throws on stale handle") {
        FetchList<int> list;
        auto handle = list.emplace(42);
        list.erase(handle);

        const FetchList<int>& const_list = list;

        // Test const at() exception path
        REQUIRE_THROWS_AS(const_list.at(handle), std::out_of_range);
    }
}
