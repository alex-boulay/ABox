#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "ShaderHandler.hpp"
#include <GLFW/glfw3.h>
/**
 * @class ABoxApp
 * @brief Vulkan Loader application
 *
 */
class ABoxApp {
  static const uint_fast16_t width  = 800;
  static const uint_fast16_t height = 600;

  ShaderHandler shaderHandler;

  void mainLoop();

   public:
  ABoxApp();
  void run();
};
