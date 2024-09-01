#include "VkTstApp.hpp"
#include <exception>
//#define GLFW_VERSION_MAJOR 3
//#define GLFW_VERSION_MINOR 3
//#define GLFW_VERSION_REVISION 8
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <cstdlib>

int main() {
  VkTstApp app;
  glfwInit();

  try {
    app.run();
  }catch (const std::exception& e){
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

