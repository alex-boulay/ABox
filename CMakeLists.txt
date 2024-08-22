cmake_minimum_required(VERSION 3.16)

#Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#compile commands for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project configuration
project(VkTstApp)

find_package(glslang REQUIRED)
find_package(SPIRV-Tools REQUIRED)

FILE(GLOB SOURCE_FILES "*.cpp")
FILE(GLOB HEADER_FILES "*.hpp")

# Target executable
add_executable(VkTstApp ${SOURCE_FILES} ${HEADER_FILES})

# Find and link fmt library
find_package(fmt REQUIRED)
target_link_libraries(VkTstApp fmt::fmt)

# Linker flags (adjust based on your target platform)
target_link_libraries(VkTstApp "-lglfw -lvulkan -ldl -lpthread -lXrandr -lXi")
