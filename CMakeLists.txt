cmake_minimum_required(VERSION 3.25)

# Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH /usr/local ${CMAKE_PREFIX_PATH})

# Enable AddressSanitizer in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
  # no C here    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror
  # -gdwarf-4 -fsanitize=address -fno-omit-frame-pointer -static-libasan")

  include(FetchContent)

  FetchContent_Declare(
    tracy
    GIT_REPOSITORY "https://github.com/wolfpld/tracy.git"
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)

  option(TRACY_ENABLE "Enable Tracy Profiler" ON)
  option(TRACY_ON_DEMAND "Enable Tracy on-demand profiling" ON)

  FetchContent_MakeAvailable(tracy)

  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -gdwarf-5  -fsanitize=address -Og -fno-omit-frame-pointer -static-libasan "
  )
  set(CMAKE_LINKER_FLAGS
      "${CMAKE_LINKER_FLAGS} -fsanitize=address -static-libasan")
endif()

# compile commands for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project configuration
project(ABox)

find_package(Threads REQUIRED)
find_package(glslang REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SPIRV-Tools REQUIRED)
find_package(glfw3 REQUIRED)

# Print environment variables to verify
message(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
message(STATUS "Vulkan_INCLUDE_DIR: $ENV{Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan_LIBRARY: $ENV{Vulkan_LIBRARY}")
message(STATUS "GLFW found at: ${GLFW_INCLUDE_DIRS}")

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

# Target executable
add_executable(ABox ${SOURCE_FILES} ${HEADER_FILES})

# Linker flags (adjust based on your target platform)
target_link_libraries(
  ABox
  PRIVATE Vulkan::Vulkan
          glslang::glslang
          glslang::SPIRV
          glfw3
          dl
          pthread
          Xrandr
          Xi
          glslang-default-resource-limits)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_link_libraries(ABox PUBLIC TracyClient)
endif()

set_target_properties(ABox PROPERTIES INSTALL_RPATH "$VULKAN_SDK/lib")
# Specify the install directory for the executable
install(TARGETS ABox RUNTIME DESTINATION bin)

# Optionally install shaders or other resources
install(DIRECTORY shaders/ DESTINATION share/ABox/shaders)
install(DIRECTORY resources/ DESTINATION share/ABox/resources)
