# ABox

## Vulkan setup for calculation and processing data
  
Main goal is to create an easy loader for shaders and configuration for ressour
ces allocations inside a Vulkan appcode base : C++ 20 - Vulkan C API

### Include libs

glslang-dev glslang-tools spirv-tools GLFW

### TODO

#### Development

- ResourcesManager
  - WaitIdle on all devices 
- Debugging Levels 
  - Global one
  - Per Frame activate/deactivate
- Compute Pipelines
  - Deploy
  - Make them able to leave alongside GraphicsPipeline
  - QueueFamilies optimisations (maxload)

#### Platform 

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
- Shader ressources handling 
  - need to manage/free them
- Wrapper extra behavior 
  - Notion of parent -> (device only atm need a template : Device/Instance)
  - Self Destruction call 
  - Specific lambda destructions catch with parents callbacks 
- AutoLoad/Manage DebugHandler
- CommandPools/ Buffers 
- Queues
