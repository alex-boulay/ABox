set(MEMORY_TEST_SOURCES
  test_memory_wrapper.cpp
)

add_executable(memory_tests ${MEMORY_TEST_SOURCES})

target_link_libraries(memory_tests
  PRIVATE
    ${LIBRARY_NAME}
    Catch2::Catch2WithMain
)

target_include_directories(memory_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

catch_discover_tests(memory_tests)

# Custom target to run memory tests only
add_custom_target(run_memory_tests
  COMMAND memory_tests --success
  DEPENDS memory_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running memory tests with Catch2"
)
