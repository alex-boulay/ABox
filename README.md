# ABox

## Vulkan setup for calculation and processing data
  
Main goal is to create an easy loader for shaders and configuration for ressour
ces allocations inside a Vulkan appcode base : C++ 20 - Vulkan C API

### Include libs

glslang-dev glslang-tools spirv-tools GLFW

### TODO

#### Code Cleanup & Refactoring

- Logging system (will address debug output issues)
  - Replace scattered std::cout with unified logging interface
  - Add log levels (DEBUG, INFO, WARN, ERROR)
  - Optional callback system for external monitoring
- Error handling improvements
  - Add VkResult checking to CommandsHandler Vulkan calls (vkCreateCommandPool, vkAllocateCommandBuffers, vkBeginCommandBuffer)
  - Verify swapchain recreation behavior without pipeline
- Dead code removal
  - Remove unused DeviceHandler::loadShader() method
  - Remove unused loadedShaders map in DeviceBoundElements
  - Remove commented graphicsppl code in DeviceHandler.hpp
- Clean up unnecessary includes
  - Review header dependencies (e.g., GraphicsPipeline.hpp in DeviceHandler.hpp)

#### Development

- Debugging Levels
  - Global one
  - Per Frame activate/deactivate
- PipelineManager improvements
  - Add pipeline naming/retrieval system validation
  - Support for multiple graphics pipelines (materials, post-processing)
- SPIRV-Reflect integration
  - Automatic descriptor set layout generation (partially done in PipelineBase)
  - Push constant reflection
  - Validation of shader interface compatibility
- QueueFamilies optimisations (maxload)

#### Platform

- Logging system
  - Replace scattered std::cout with unified logging interface
  - Add log levels (DEBUG, INFO, WARN, ERROR)
  - Optional callback system for external monitoring
- load tracy :
  - mem tracking
  - performance

### Do I do ?

- VMA ?
- Custom Memmory allocators overloaded with tracy for CPU too ?
- Make a config file ? - Handle external compilation ?
- inlude ImGui loading interface ?
- Make everything pickable from a menu ? then save it into a config xml/json ?

### Done

- Automanage Malloc/Free Vulkan Structure dependencies
- tracy part of the makefile (might have to change the download to local archive for stability and offline purposes)
- VkShader Loading and binding
- ResourcesManager Instance Loading
- Device Manager
- GraphicsPipeline
- Swapchain
  - Swapchain recreation
- Shader ressources handling
  - need to manage/free them
- Wrapper extra behavior
  - Notion of parent -> (device only atm need a template : Device/Instance)
  - Self Destruction call
  - Specific lambda destructions catch with parents callbacks
- AutoLoad/Manage DebugHandler
- CommandPools/ Buffers
- Queues
- ResourcesManager
  - WaitIdle on all devices
- Add clang-format file to avoid code moving around when commiting from different IDE
- Compute Pipelines
  - Deploy ComputePipeline class
  - Make them able to coexist alongside GraphicsPipeline via PipelineManager
- PipelineManager
  - Unified management of Graphics, Compute, and RayTracing pipelines
  - Variant-based heterogeneous storage without heap allocation
  - C++20 ranges support for shader loading
