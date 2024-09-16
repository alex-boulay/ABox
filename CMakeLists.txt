cmake_minimum_required(VERSION 3.25)

#Cpp standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AddressSanitizer in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -g -fsanitize=address -fno-omit-frame-pointer -static-libasan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -g -fsanitize=address -fno-omit-frame-pointer -static-libasan")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address -static-libasan")
endif()

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

# Linker flags (adjust based on your target platform)
target_link_libraries(VkTstApp PRIVATE "-lglfw -lvulkan -ldl -lpthread -lXrandr -lXi -lglslang-default-resource-limits" glslang::glslang glslang::SPIRV)

# Specify the install directory for the executable
install(TARGETS VkTstApp
        RUNTIME DESTINATION bin)

# Optionally install shaders or other resources
install(DIRECTORY shaders/ DESTINATION share/VkTstApp/shaders)
install(DIRECTORY resources/ DESTINATION share/VkTstApp/resources)
