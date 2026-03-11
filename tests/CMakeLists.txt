message(STATUS "----------------------------------------------------------")
message(STATUS "Building unit tests")
message(STATUS "----------------------------------------------------------")

# Code coverage support (configured from root CMakeLists.txt)
if(ENABLE_COVERAGE)
  message(STATUS "Code coverage ENABLED")
  # Apply coverage flags to the library
  target_compile_options(${LIBRARY_NAME} PUBLIC --coverage)
  target_link_options(${LIBRARY_NAME} PUBLIC --coverage)
endif()

# Fetch Catch2
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(Catch2)

# Apply coverage flags to all test targets (after Catch2 is fetched)
if(ENABLE_COVERAGE)
  add_compile_options(--coverage)
  add_link_options(--coverage)
endif()

# Add Catch2 CMake modules to path
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Enable testing (owned by tests/)
enable_testing()
include(CTest)
include(Catch)

# List of test subdirectories (matching src/ structure)
set(TEST_SUBDIRS
    utils
    vulkan
    graphics
    window
    memory
    core
    pipelines)

# Add subdirectories that exist
foreach(SUBDIR ${TEST_SUBDIRS})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
    add_subdirectory(${SUBDIR})
  endif()
endforeach()

# Custom target to build and run all tests
add_custom_target(
  test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running all unit tests"
  USES_TERMINAL)

# Coverage target is defined in root CMakeLists.txt
