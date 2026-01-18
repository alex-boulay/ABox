option(BUILD_ABOX_APP "Build the main ABox application" ON)
option(BUILD_ALLOCATOR_TEST "Build the allocator chunking test utility" ON)

if(BUILD_ABOX_APP)
  message(STATUS "Building ABox application")
  add_subdirectory(abox)
endif()

if(BUILD_ALLOCATOR_TEST)
  message(STATUS "Building allocator test utility")
  add_subdirectory(allocator_test)
endif()
