cmake_minimum_required(VERSION 3.25)

# Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH /usr/local ${CMAKE_PREFIX_PATH})

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
add_compile_definitions(SHADER_DIR="${SHADER_DIR}")

if(CMAKE_BUILD_TYPE MATCHES Debug)

  include(FetchContent)

  #   FetchContent_Declare(
  #   tracy
  #   GIT_REPOSITORY "https://github.com/wolfpld/tracy.git"
  #   GIT_TAG master
  #   GIT_SHALLOW TRUE
  #   GIT_PROGRESS TRUE)

  # option(TRACY_ENABLE "Enable Tracy Profiler" ON)
  # option(TRACY_ON_DEMAND "Enable Tracy on-demand profiling" ON)

  #FetchContent_MakeAvailable(tracy)

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

find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(glslang REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SPIRV-Tools REQUIRED)
find_package(SPIRV-Reflect REQUIRED)

# Print environment variables to verify
message(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
message(STATUS "Vulkan_INCLUDE_DIR: $ENV{Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan_LIBRARY: $ENV{Vulkan_LIBRARY}")

message(STATUS "GLFW3 libraries: ${GLFW3_LIBRARIES}")
message(STATUS "GLFW3 library: ${GLFW3_LIBRARY}")
message(STATUS "glfw3 libraries: ${glfw3_LIBRARIES}")
message(STATUS "glfw3 library: ${glfw3_LIBRARY}")
message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
message(STATUS "GLFW_LIBRARY: ${GLFW_LIBRARY}")
message(STATUS "GLFW3 include dirs: ${GLFW3_INCLUDE_DIRS}")
message(STATUS "GLFW3 include dir: ${GLFW3_INCLUDE_DIR}")
message(STATUS "glfw3 include dirs: ${glfw3_INCLUDE_DIRS}")
message(STATUS "GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")

# Recursively gather all source and header files from src/
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS src/*.hpp)

add_executable(ABox ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(
  ABox
  PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/graphics
          ${CMAKE_SOURCE_DIR}/src/vulkan ${CMAKE_SOURCE_DIR}/src/window
          ${CMAKE_SOURCE_DIR}/src/memory ${CMAKE_SOURCE_DIR}/src/utils
          ${CMAKE_SOURCE_DIR}/src/pipelines)

target_link_libraries(
  ABox
  PUBLIC Vulkan::Vulkan
         glslang::glslang
         glslang::SPIRV
         SPIRV-Reflect::spirv-reflect-static
         glfw3
         dl
         pthread
         Xrandr
         Xi
         glslang-default-resource-limits)

#if(CMAKE_BUILD_TYPE MATCHES Debug)
#  target_link_libraries(ABox PUBLIC TracyClient)
#endif()

set_target_properties(ABox PROPERTIES INSTALL_RPATH "$VULKAN_SDK/lib")
# Specify the install directory for the executable
install(TARGETS ABox RUNTIME DESTINATION bin)

# Optionally install shaders or other resources
install(DIRECTORY shaders/ DESTINATION share/ABox/shaders)
install(DIRECTORY resources/ DESTINATION share/ABox/resources)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_BINARY_DIR}/compile_commands.json"
          "${CMAKE_SOURCE_DIR}/compile_commands.json"
)
