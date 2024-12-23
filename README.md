# ABox

## Vulkan setup for calculation and processing data
  
Main goal is to create an easy loader for shaders and configuration for ressour
ces allocations inside a Vulkan appcode base : C++ 20 - Vulkan C API

### Include libs

glslang-dev glslang-tools spirv-tools GLFW

### TODO

- enhance device picking ?? :
  - prompt to choose physical device.
- VkShader Loading and binding
- RessourcesManager Instance Loading :
  - Finalising Layers
  - Finalising extensions

### Do I do ?

- Make a config file ? - Handle external compilation ?
- inlude ImGui loading interface ?
- Make everything pickable from a menu ? then save it into a config xml/json ?
- Do I automanage Malloc/Free Vulkan Structure dependencies  
