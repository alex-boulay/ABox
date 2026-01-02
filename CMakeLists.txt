cmake_minimum_required(VERSION 3.25)


option(BUILD_APP "Build the executable application" OFF)
if(NOT BUILD_APP)
  message(STATUS "----------------------------------------------------------")
  message(STATUS "NOTE: Building in LIBRARY mode.")
  message(STATUS "Run 'cmake -DBUILD_APP=ON ..' to build the executable app!")
  message(STATUS "----------------------------------------------------------")
else()
  message(STATUS "Building in EXECUTABLE mode.")
endif()

set(EXECUTABLE_NAME ABox)
set(LIBRARY_NAME ABoxLib)

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

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS src/*.hpp)

set(EXCLUDE_NAMES "main.cpp" "AboxApp.cpp" "AboxApp.hpp")

foreach(FILE_PATH ${ALL_SOURCES} ${ALL_HEADERS})
  get_filename_component(FILE_NAME ${FILE_PATH} NAME)
  if(${FILE_NAME} IN_LIST EXCLUDE_NAMES)
    list(APPEND APP_FILES ${FILE_PATH})
  else()
    list(APPEND LIB_FILES ${FILE_PATH})
  endif()
endforeach()

add_library(${LIBRARY_NAME} ${LIB_FILES})
target_include_directories(${LIBRARY_NAME} PUBLIC src)

target_compile_definitions(
  ${LIBRARY_NAME}
  PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
)

target_include_directories(
  ${LIBRARY_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/graphics
          ${CMAKE_SOURCE_DIR}/src/vulkan ${CMAKE_SOURCE_DIR}/src/window
          ${CMAKE_SOURCE_DIR}/src/memory ${CMAKE_SOURCE_DIR}/src/utils
          ${CMAKE_SOURCE_DIR}/src/pipelines)

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

if(BUILD_APP)
  message(STATUS "Building in EXE mode...")

  add_executable(${EXECUTABLE_NAME} ${APP_FILES})

  target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/core ${CMAKE_SOURCE_DIR}/src/graphics
            ${CMAKE_SOURCE_DIR}/src/vulkan ${CMAKE_SOURCE_DIR}/src/window
            ${CMAKE_SOURCE_DIR}/src/memory ${CMAKE_SOURCE_DIR}/src/utils
            ${CMAKE_SOURCE_DIR}/src/pipelines
  )
  target_compile_definitions(
    ${EXECUTABLE_NAME}
    PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
  )
  target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LIBRARY_NAME})

  install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
  install(DIRECTORY shaders/ DESTINATION share/ABox/shaders)
  install(DIRECTORY resources/ DESTINATION share/ABox/resources)

  set_target_properties(ABox PROPERTIES INSTALL_RPATH "$VULKAN_SDK/lib")

  add_custom_command(
  TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_BINARY_DIR}/compile_commands.json"
          "${CMAKE_SOURCE_DIR}/compile_commands.json"
  )
else()
  message(STATUS "Building in LIBRARY-ONLY mode...")
  add_custom_command(
    TARGET ${LIBRARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compilation database to source root for LSP support"
  )
endif()




