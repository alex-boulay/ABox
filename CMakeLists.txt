cmake_minimum_required(VERSION 3.16)

#Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#compile commands for lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project configuration
project(VkTstApp)

find_package(Threads REQUIRED)
find_package(glslang REQUIRED)
find_package(SPIRV-Tools REQUIRED)

FILE(GLOB SOURCE_FILES "*.cpp")
FILE(GLOB HEADER_FILES "*.hpp")

# Target executable
add_executable(VkTstApp ${SOURCE_FILES} ${HEADER_FILES})

#compile warning flags
target_compile_options(VkTstApp PRIVATE -Wall -Wextra -Werror)

# Linker flags (adjust based on your target platform)
target_link_libraries(VkTstApp PRIVATE "-lglfw -lvulkan -ldl -lpthread -lXrandr -lXi -lglslang-default-resource-limits" glslang::glslang glslang::SPIRV)

# Specify the install directory for the executable
install(TARGETS VkTstApp
        RUNTIME DESTINATION bin)

# Optionally install shaders or other resources
install(DIRECTORY shaders/ DESTINATION share/VkTstApp/shaders)
install(DIRECTORY resources/ DESTINATION share/VkTstApp/resources)
