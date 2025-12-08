#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "ResourcesManager.hpp"
#include "ShaderHandler.hpp"
#include "WindowManager.hpp"
#include <GLFW/glfw3.h>

// #include "ShaderHandler.hpp"
/**
 * @class ABoxApp
 * @brief Vulkan Loader application
 *
 */
class ABoxApp {
  static constexpr VkExtent2D baseWindowDimention = {
      .width  = 800u,
      .height = 600u
  };

  WindowManager    wm{baseWindowDimention};
  ResourcesManager rs;
  ShaderHandler    shaderHandler;

   public:
  ABoxApp();
  ~ABoxApp();
  void run();
};
