#include <catch2/catch_test_macros.hpp>
#include <MemoryWrapper.hpp>
#include <atomic>

// Mock destruction functions for testing
static std::atomic<int> solo_destructor_calls{0};
static std::atomic<int> parent_destructor_calls{0};

// Mock types for testing (not real Vulkan handles)
using MockHandle = void*;
using MockDevice = void*;

// Mock destruction functions
void MockDestroySolo(MockHandle handle, const VkAllocationCallbacks* /*allocator*/) {
    if (handle != VK_NULL_HANDLE) {
        solo_destructor_calls++;
    }
}

void MockDestroyWithParent(MockDevice device, MockHandle handle, const VkAllocationCallbacks* /*allocator*/) {
    if (handle != VK_NULL_HANDLE && device != VK_NULL_HANDLE) {
        parent_destructor_calls++;
    }
}

// Helper to create a mock handle (non-null)
MockHandle createMockHandle() {
    return reinterpret_cast<MockHandle>(0x1234);
}

MockDevice createMockDevice() {
    return reinterpret_cast<MockDevice>(0x5678);
}

TEST_CASE("MemoryWrapper: Basic construction and destruction", "[memory][wrapper]") {
    solo_destructor_calls = 0;

    SECTION("Solo wrapper with valid handle") {
        {
            MockHandle handle = createMockHandle();
            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper(
                handle,
                nullptr,
                MockDestroySolo
            );

            REQUIRE(wrapper.get() == handle);
            REQUIRE(wrapper.empty() == true);  // empty() returns true if NOT null
        }
        // Destructor should have been called
        REQUIRE(solo_destructor_calls == 1);
    }

    SECTION("Solo wrapper with VK_NULL_HANDLE") {
        solo_destructor_calls = 0;
        {
            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper(
                VK_NULL_HANDLE,
                nullptr,
                MockDestroySolo
            );

            REQUIRE(wrapper.get() == VK_NULL_HANDLE);
        }
        // Destructor should NOT be called for null handle
        REQUIRE(solo_destructor_calls == 0);
    }
}

TEST_CASE("MemoryWrapper: Parent-based construction and destruction", "[memory][wrapper]") {
    parent_destructor_calls = 0;

    SECTION("Wrapper with parent and valid handle") {
        {
            MockHandle handle = createMockHandle();
            MockDevice device = createMockDevice();

            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper(
                handle,
                device,
                MockDestroyWithParent
            );

            REQUIRE(wrapper.get() == handle);
        }
        // Destructor should have been called
        REQUIRE(parent_destructor_calls == 1);
    }

    SECTION("Wrapper with null parent should not call destructor") {
        parent_destructor_calls = 0;
        {
            MockHandle handle = createMockHandle();

            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper(
                handle,
                VK_NULL_HANDLE,  // null parent
                MockDestroyWithParent
            );

            REQUIRE(wrapper.get() == handle);
        }
        // Destructor should NOT be called when parent is null
        REQUIRE(parent_destructor_calls == 0);
    }
}

TEST_CASE("MemoryWrapper: Move semantics", "[memory][wrapper]") {
    SECTION("Move constructor") {
        solo_destructor_calls = 0;

        {
            MockHandle handle = createMockHandle();

            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper1(
                handle,
                nullptr,
                MockDestroySolo
            );

            // Move construct wrapper2 from wrapper1
            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper2(std::move(wrapper1));

            // wrapper2 should have the handle
            REQUIRE(wrapper2.get() == handle);
            // wrapper1 should be null
            REQUIRE(wrapper1.get() == VK_NULL_HANDLE);
        }
        // Only one destructor call should happen (from wrapper2)
        REQUIRE(solo_destructor_calls == 1);
    }

    SECTION("Move assignment") {
        solo_destructor_calls = 0;

        {
            MockHandle handle1 = createMockHandle();
            MockHandle handle2 = reinterpret_cast<MockHandle>(0xABCD);

            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper1(
                handle1,
                nullptr,
                MockDestroySolo
            );

            MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper2(
                handle2,
                nullptr,
                MockDestroySolo
            );

            // Move assign wrapper1 to wrapper2
            wrapper2 = std::move(wrapper1);

            // wrapper2 should now have handle1
            REQUIRE(wrapper2.get() == handle1);
            // wrapper1 should be null
            REQUIRE(wrapper1.get() == VK_NULL_HANDLE);
        }
        // wrapper2 should destroy handle1 when going out of scope
        // No destructor call for wrapper1 since it was moved-from (nulled out)
        REQUIRE(solo_destructor_calls == 1);
    }
}

TEST_CASE("MemoryWrapper: Accessors", "[memory][wrapper]") {
    MockHandle handle = createMockHandle();

    MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper(
        handle,
        nullptr,
        MockDestroySolo
    );

    SECTION("get() returns the handle") {
        REQUIRE(wrapper.get() == handle);
    }

    SECTION("ptr() returns pointer to handle") {
        REQUIRE(wrapper.ptr() != nullptr);
        REQUIRE(*wrapper.ptr() == handle);
    }

    SECTION("Implicit conversion operator") {
        MockHandle retrieved = wrapper;
        REQUIRE(retrieved == handle);
    }
}

TEST_CASE("MemoryWrapper: swap() functionality", "[memory][wrapper]") {
    solo_destructor_calls = 0;

    MockHandle handle1 = createMockHandle();
    MockHandle handle2 = reinterpret_cast<MockHandle>(0x9999);

    MemoryWrapper<MockHandle, decltype(&MockDestroySolo)> wrapper(
        handle1,
        nullptr,
        MockDestroySolo
    );

    // Swap should destroy old handle and replace with new
    wrapper.swap(handle2);

    REQUIRE(wrapper.get() == handle2);
    // One destructor call for handle1
    REQUIRE(solo_destructor_calls == 1);
}
