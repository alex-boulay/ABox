# Platform abstraction library
cmake_minimum_required(VERSION 3.25)

# Collect headers
file(GLOB PLATFORM_HEADERS CONFIGURE_DEPENDS *.hpp)

# Select platform-specific sources
if(UNIX AND NOT APPLE)
  # Linux
  message(STATUS "Platform: Linux - using native futex")
  file(GLOB PLATFORM_SOURCES linux/*.cpp)
elseif(WIN32)
  # Windows
  message(STATUS "Platform: Windows - using WaitOnAddress")
  file(GLOB PLATFORM_SOURCES windows/*.cpp)
else()
  # Fallback (macOS, BSD, etc)
  message(STATUS "Platform: Fallback - using spin-wait")
  file(GLOB PLATFORM_SOURCES fallback/*.cpp)
endif()

# Create platform library
add_library(ABoxPlatform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

# Export include directory
target_include_directories(ABoxPlatform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
