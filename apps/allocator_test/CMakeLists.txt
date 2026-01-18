file(GLOB_RECURSE ALLOCATOR_TEST_SOURCES *.cpp)

add_executable(allocator_test ${ALLOCATOR_TEST_SOURCES})

install(TARGETS allocator_test RUNTIME DESTINATION bin)
