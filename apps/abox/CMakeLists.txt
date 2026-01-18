set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../shaders")

file(GLOB_RECURSE ABOX_APP_SOURCES *.cpp)
file(GLOB_RECURSE ABOX_APP_HEADERS *.hpp)

add_executable(ABox ${ABOX_APP_SOURCES} ${ABOX_APP_HEADERS})

target_include_directories(
  ABox
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/graphics
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/vulkan
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/window
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/memory
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils
          ${CMAKE_CURRENT_SOURCE_DIR}/../../src/pipelines
)

target_compile_definitions(
  ABox
  PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
  PRIVATE SHADER_DIR="${SHADER_DIR}"
)

target_link_libraries(ABox PRIVATE ABoxLib)

set_target_properties(ABox PROPERTIES INSTALL_RPATH "$VULKAN_SDK/lib")

install(TARGETS ABox RUNTIME DESTINATION bin)
