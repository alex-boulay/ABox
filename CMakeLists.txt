cmake_minimum_required(VERSION 3.25)


option(BUILD_APPS "Build executable applications" OFF)

set(LIBRARY_NAME ABoxLib)

# Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH /usr/local ${CMAKE_PREFIX_PATH})


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

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS src/*.hpp)

add_library(${LIBRARY_NAME} ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(${LIBRARY_NAME} PUBLIC src)

target_compile_definitions(
  ${LIBRARY_NAME}
  PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
)

target_include_directories(
  ${LIBRARY_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics
          ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan ${CMAKE_CURRENT_SOURCE_DIR}/src/window
          ${CMAKE_CURRENT_SOURCE_DIR}/src/memory ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
          ${CMAKE_CURRENT_SOURCE_DIR}/src/pipelines)

target_link_libraries(
  ${LIBRARY_NAME}
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

install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY src/
        DESTINATION include/ABox
        FILES_MATCHING PATTERN "*.hpp")

if(BUILD_APPS)
  message(STATUS "----------------------------------------------------------")
  message(STATUS "Building applications (see apps/CMakeLists.txt for options)")
  message(STATUS "----------------------------------------------------------")
  add_subdirectory(apps)

  install(DIRECTORY shaders/ DESTINATION share/ABox/shaders)
  install(DIRECTORY resources/ DESTINATION share/ABox/resources)
else()
  message(STATUS "----------------------------------------------------------")
  message(STATUS "Building LIBRARY only")
  message(STATUS "Run 'cmake -DBUILD_APPS=ON ..' to build applications")
  message(STATUS "----------------------------------------------------------")
endif()

add_custom_command(
  TARGET ${LIBRARY_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_BINARY_DIR}/compile_commands.json"
          "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json"
  COMMENT "Copying compilation database to source root for LSP support"
)




