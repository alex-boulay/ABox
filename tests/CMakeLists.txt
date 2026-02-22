cmake_minimum_required(VERSION 3.25)

# Fetch Catch2
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Catch2)

# Add Catch2 CMake modules to path
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Collect all test files from subdirectories
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
  utils/*.cpp
  vulkan/*.cpp
  graphics/*.cpp
  window/*.cpp
  memory/*.cpp
  core/*.cpp
  pipelines/*.cpp
)

# Only build tests if we have test files
if(TEST_SOURCES)
  add_executable(abox_tests ${TEST_SOURCES})

  target_link_libraries(abox_tests
    PRIVATE
      ${LIBRARY_NAME}
      Catch2::Catch2WithMain
  )

  target_include_directories(abox_tests
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/../src
  )

  # Enable Catch2's CMake integration for CTest
  include(CTest)
  include(Catch)
  catch_discover_tests(abox_tests)

  # Add custom target to run tests with verbose output
  add_custom_target(run_tests
    COMMAND abox_tests --success
    DEPENDS abox_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running unit tests with Catch2"
  )
else()
  message(STATUS "No test files found yet. Add tests in tests/ subdirectories.")
endif()
