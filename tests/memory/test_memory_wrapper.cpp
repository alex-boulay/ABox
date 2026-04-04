#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
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

TEST_CASE("MemoryWrapper: Move semantics with parent", "[memory][wrapper]") {
    SECTION("Move constructor with parent logs correctly") {
        parent_destructor_calls = 0;

        {
            MockHandle handle = createMockHandle();
            MockDevice device = createMockDevice();

            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper1(
                handle,
                device,
                MockDestroyWithParent
            );

            // Move construct wrapper2 from wrapper1
            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper2(std::move(wrapper1));

            // wrapper2 should have the handle and parent
            REQUIRE(wrapper2.get() == handle);
            // wrapper1 should be null
            REQUIRE(wrapper1.get() == VK_NULL_HANDLE);
        }
        // Only one destructor call should happen (from wrapper2)
        REQUIRE(parent_destructor_calls == 1);
    }

    SECTION("Move assignment with parent logs correctly") {
        parent_destructor_calls = 0;

        {
            MockHandle handle1 = createMockHandle();
            MockDevice device1 = createMockDevice();
            MockHandle handle2 = reinterpret_cast<MockHandle>(0xABCD);
            MockDevice device2 = reinterpret_cast<MockDevice>(0xDEF0);

            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper1(
                handle1,
                device1,
                MockDestroyWithParent
            );

            MemoryWrapper<MockHandle, decltype(&MockDestroyWithParent), MockDevice> wrapper2(
                handle2,
                device2,
                MockDestroyWithParent
            );

            // Move assign wrapper1 to wrapper2
            wrapper2 = std::move(wrapper1);

            // wrapper2 should now have handle1
            REQUIRE(wrapper2.get() == handle1);
            // wrapper1 should be null
            REQUIRE(wrapper1.get() == VK_NULL_HANDLE);
        }
        // wrapper2 should destroy handle1 when going out of scope
        REQUIRE(parent_destructor_calls == 1);
    }
}

// Template-based tests to hit all function instantiations
// Define pointer types for template testing
struct MockHandle64 {};
struct MockHandle32 {};

template<typename T>
struct MockHandleTraits;

template<>
struct MockHandleTraits<void*> {
    using HandleType = void*;
    using DeviceType = void*;
    static constexpr HandleType null_handle = VK_NULL_HANDLE;
    static constexpr DeviceType null_device = VK_NULL_HANDLE;
    static HandleType createHandle() { return reinterpret_cast<void*>(0x1234); }
    static DeviceType createDevice() { return reinterpret_cast<void*>(0x5678); }
    static HandleType createHandle2() { return reinterpret_cast<void*>(0xABCD); }
    static DeviceType createDevice2() { return reinterpret_cast<void*>(0xDEF0); }
    static HandleType createHandle3() { return reinterpret_cast<void*>(0x9999); }
};

template<>
struct MockHandleTraits<MockHandle64*> {
    using HandleType = MockHandle64*;
    using DeviceType = MockHandle64*;
    static constexpr HandleType null_handle = VK_NULL_HANDLE;
    static constexpr DeviceType null_device = VK_NULL_HANDLE;
    static HandleType createHandle() { return reinterpret_cast<MockHandle64*>(0x123456789ABCDEF0); }
    static DeviceType createDevice() { return reinterpret_cast<MockHandle64*>(0x1122334455667788); }
    static HandleType createHandle2() { return reinterpret_cast<MockHandle64*>(0xABCD); }
    static DeviceType createDevice2() { return reinterpret_cast<MockHandle64*>(0xDEF0); }
    static HandleType createHandle3() { return reinterpret_cast<MockHandle64*>(0x9999); }
};

template<>
struct MockHandleTraits<MockHandle32*> {
    using HandleType = MockHandle32*;
    using DeviceType = MockHandle32*;
    static constexpr HandleType null_handle = VK_NULL_HANDLE;
    static constexpr DeviceType null_device = VK_NULL_HANDLE;
    static HandleType createHandle() { return reinterpret_cast<MockHandle32*>(0x12345678); }
    static DeviceType createDevice() { return reinterpret_cast<MockHandle32*>(0x87654321); }
    static HandleType createHandle2() { return reinterpret_cast<MockHandle32*>(0xABCD); }
    static DeviceType createDevice2() { return reinterpret_cast<MockHandle32*>(0xDEF0); }
    static HandleType createHandle3() { return reinterpret_cast<MockHandle32*>(0x9999); }
};

static std::atomic<int> template_destructor_solo{0};
static std::atomic<int> template_destructor_parent{0};

template<typename T>
void TemplateDestroySolo(T handle, const VkAllocationCallbacks* /*allocator*/) {
    if (handle != MockHandleTraits<T>::null_handle) {
        template_destructor_solo++;
    }
}

template<typename T>
void TemplateDestroyWithParent(T device, T handle, const VkAllocationCallbacks* /*allocator*/) {
    if (handle != MockHandleTraits<T>::null_handle && device != MockHandleTraits<T>::null_device) {
        template_destructor_parent++;
    }
}

TEMPLATE_TEST_CASE("MemoryWrapper: Template instantiation coverage", "[memory][wrapper][template]", void*, MockHandle64*, MockHandle32*) {
    using Traits = MockHandleTraits<TestType>;
    using HandleType = typename Traits::HandleType;
    using DeviceType = typename Traits::DeviceType;

    SECTION("Solo wrapper construction and destruction") {
        template_destructor_solo = 0;

        {
            HandleType handle = Traits::createHandle();
            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper(
                handle,
                nullptr,
                TemplateDestroySolo<HandleType>
            );

            REQUIRE(wrapper.get() == handle);
            REQUIRE(wrapper.empty() == true);
        }

        REQUIRE(template_destructor_solo == 1);
    }

    SECTION("Wrapper with parent") {
        template_destructor_parent = 0;

        {
            HandleType handle = Traits::createHandle();
            DeviceType device = Traits::createDevice();

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper(
                handle,
                device,
                TemplateDestroyWithParent<HandleType>
            );

            REQUIRE(wrapper.get() == handle);
        }

        REQUIRE(template_destructor_parent == 1);
    }

    SECTION("Move constructor solo") {
        template_destructor_solo = 0;

        {
            HandleType handle = Traits::createHandle();

            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper1(
                handle,
                nullptr,
                TemplateDestroySolo<HandleType>
            );

            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper2(std::move(wrapper1));

            REQUIRE(wrapper2.get() == handle);
            REQUIRE(wrapper1.get() == Traits::null_handle);
        }

        REQUIRE(template_destructor_solo == 1);
    }

    SECTION("Move assignment solo") {
        template_destructor_solo = 0;

        {
            HandleType handle1 = Traits::createHandle();
            HandleType handle2 = Traits::createHandle2();

            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper1(
                handle1,
                nullptr,
                TemplateDestroySolo<HandleType>
            );

            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper2(
                handle2,
                nullptr,
                TemplateDestroySolo<HandleType>
            );

            wrapper2 = std::move(wrapper1);

            REQUIRE(wrapper2.get() == handle1);
            REQUIRE(wrapper1.get() == Traits::null_handle);
        }

        REQUIRE(template_destructor_solo == 1);
    }

    SECTION("Move constructor with parent") {
        template_destructor_parent = 0;

        {
            HandleType handle = Traits::createHandle();
            DeviceType device = Traits::createDevice();

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper1(
                handle,
                device,
                TemplateDestroyWithParent<HandleType>
            );

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper2(std::move(wrapper1));

            REQUIRE(wrapper2.get() == handle);
            REQUIRE(wrapper1.get() == Traits::null_handle);
        }

        REQUIRE(template_destructor_parent == 1);
    }

    SECTION("Move assignment with parent") {
        template_destructor_parent = 0;

        {
            HandleType handle1 = Traits::createHandle();
            DeviceType device1 = Traits::createDevice();
            HandleType handle2 = Traits::createHandle2();
            DeviceType device2 = Traits::createDevice2();

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper1(
                handle1,
                device1,
                TemplateDestroyWithParent<HandleType>
            );

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper2(
                handle2,
                device2,
                TemplateDestroyWithParent<HandleType>
            );

            wrapper2 = std::move(wrapper1);

            REQUIRE(wrapper2.get() == handle1);
            REQUIRE(wrapper1.get() == Traits::null_handle);
        }

        REQUIRE(template_destructor_parent == 1);
    }

    SECTION("Accessors") {
        HandleType handle = Traits::createHandle();

        MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper(
            handle,
            nullptr,
            TemplateDestroySolo<HandleType>
        );

        REQUIRE(wrapper.get() == handle);
        REQUIRE(wrapper.ptr() != nullptr);
        REQUIRE(*wrapper.ptr() == handle);

        HandleType retrieved = wrapper;
        REQUIRE(retrieved == handle);
    }

    SECTION("Swap functionality") {
        template_destructor_solo = 0;

        HandleType handle1 = Traits::createHandle();
        HandleType handle2 = Traits::createHandle3();

        MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper(
            handle1,
            nullptr,
            TemplateDestroySolo<HandleType>
        );

        wrapper.swap(handle2);

        REQUIRE(wrapper.get() == handle2);
        REQUIRE(template_destructor_solo == 1);
    }

    SECTION("Null handle does not call destructor") {
        template_destructor_solo = 0;

        {
            MemoryWrapper<HandleType, decltype(&TemplateDestroySolo<HandleType>)> wrapper(
                Traits::null_handle,
                nullptr,
                TemplateDestroySolo<HandleType>
            );

            REQUIRE(wrapper.get() == Traits::null_handle);
        }

        REQUIRE(template_destructor_solo == 0);
    }

    SECTION("Null parent does not call destructor") {
        template_destructor_parent = 0;

        {
            HandleType handle = Traits::createHandle();

            MemoryWrapper<HandleType, decltype(&TemplateDestroyWithParent<HandleType>), DeviceType> wrapper(
                handle,
                Traits::null_device,
                TemplateDestroyWithParent<HandleType>
            );

            REQUIRE(wrapper.get() == handle);
        }

        REQUIRE(template_destructor_parent == 0);
    }
}
