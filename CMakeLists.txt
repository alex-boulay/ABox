cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE(GLOB SOURCE_FILES "*.cpp")
FILE(GLOB HEADER_FILES "*.hpp")

# Project configuration
project(VkTstApp)

# Target executable
add_executable(VkTstApp ${SOURCE_FILES} ${HEADER_FILES})

# Linker flags (adjust based on your target platform)
target_link_libraries(VkTstApp "-lglfw -lvulkan -ldl -lpthread -lXrandr -lXi")
