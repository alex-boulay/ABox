# Utils tests
set(UTILS_TEST_SOURCES
    test_versioned_slot.cpp
)

# Create test executable
add_executable(test_utils ${UTILS_TEST_SOURCES})

# Link against the library and Catch2
target_link_libraries(test_utils PRIVATE ${LIBRARY_NAME} Catch2::Catch2WithMain)

# Register with CTest
catch_discover_tests(test_utils)
