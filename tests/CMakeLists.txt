message(STATUS "----------------------------------------------------------")
message(STATUS "Building unit tests")
message(STATUS "----------------------------------------------------------")

# Code coverage support (optional)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

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
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Catch2)

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
  pipelines
)

# Add subdirectories that exist
foreach(SUBDIR ${TEST_SUBDIRS})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
    add_subdirectory(${SUBDIR})
  endif()
endforeach()

# Custom target to build and run all tests
add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running all unit tests"
  USES_TERMINAL
)

# Coverage report generation target
if(ENABLE_COVERAGE)
  find_program(LCOV_PATH lcov)
  find_program(GENHTML_PATH genhtml)

  if(LCOV_PATH AND GENHTML_PATH)
    set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
    add_custom_target(coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      COMMAND ${LCOV_PATH} --capture --directory ${CMAKE_BINARY_DIR} --output-file coverage.info --rc branch_coverage=1 --ignore-errors inconsistent
      COMMAND ${LCOV_PATH} --extract coverage.info "${SRC_DIR}/*" --output-file coverage_filtered.info --rc branch_coverage=1 --ignore-errors inconsistent,unused
      COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage-report --title "ABox Coverage" --rc branch_coverage=1 --ignore-errors inconsistent
      COMMAND ${LCOV_PATH} --summary coverage_filtered.info
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating code coverage report"
      VERBATIM
    )
    message(STATUS "Coverage target available: 'cmake --build build -t coverage'")
  else()
    message(WARNING "lcov/genhtml not found - coverage target disabled")
  endif()
endif()
