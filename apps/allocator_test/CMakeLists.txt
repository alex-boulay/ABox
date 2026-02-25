file(GLOB_RECURSE ALLOCATOR_TEST_SOURCES *.cpp)

add_executable(allocator_test ${ALLOCATOR_TEST_SOURCES})

set_target_properties(allocator_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

install(TARGETS allocator_test RUNTIME DESTINATION bin)
