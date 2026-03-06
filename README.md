# ABox

A modern Vulkan-based rendering engine written in C++20, focused on automatic resource management, custom memory allocators, and lock-free data structures.

## Features

### Core Infrastructure
- **MemoryWrapper**: RAII-based automatic Vulkan resource management with parent-child relationships
- **ResourcesManager**: Centralized Vulkan resource lifecycle management
- **DeviceHandler**: Automatic device and physical device management with extension handling

### Rendering Pipeline
- **PipelineManager**: Unified management of Graphics, Compute, and RayTracing pipelines
- **SPIRV-Reflect Integration**: Automatic descriptor set layout generation and shader reflection
- **ShaderHandler**: Automatic GLSL to SPIR-V compilation and shader module management
- **SwapchainManager**: Swapchain creation with automatic recreation on window resize
- **FrameBufferBroker**: Framebuffer lifecycle management

### Utilities
- **VersionedSlot**: Lock-free versioned slot management with futex-based synchronization
- **FetchList**: Colony-style allocator with stable pointers and version tracking
- **Logger**: Category-based logging system with configurable levels
- **Cross-platform futex abstraction**: Platform-independent synchronization primitives

### Testing
- Catch2 v3 integration with CTest
- Comprehensive test suites for MemoryWrapper and VersionedSlot
- Per-module test targets with BUILD_TESTS CMake option

## Requirements

### System Dependencies
- **Vulkan SDK**: Latest version from LunarG
- **GLFW**: Window management and input
- **glslang**: GLSL to SPIR-V compilation
- **SPIRV-Tools**: SPIR-V validation and optimization

### Build Tools
- CMake 3.15+
- C++20 compatible compiler (GCC 11+, Clang 14+)
- Ninja (recommended) or Make

## Building

```bash
# Clone the repository
git clone https://github.com/alex-boulay/ABox.git
cd ABox

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Optional: Build with tests
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build --parallel

# Run tests
ctest --test-dir build --output-on-failure
```

## Project Structure

```
ABox/
├── apps/           # Application executables
├── src/            # Library source code
│   ├── core/       # Core resource management
│   ├── graphics/   # Graphics-specific components
│   ├── memory/     # Memory management utilities
│   ├── pipelines/  # Pipeline implementations
│   ├── platform/   # Platform-specific code
│   ├── utils/      # Utility containers and helpers
│   ├── vulkan/     # Vulkan abstraction layer
│   └── window/     # Window management
└── tests/          # Test suites
    └── utils/      # Utility tests
```

## Roadmap

### In Progress
- **FetchList**: Complete integration with VersionedSlot for production use
- **Testing**: Expand test coverage to Logger and Vulkan components

### Planned
- **Documentation**: Architecture diagrams and API documentation
- **Performance**: Tracy profiler integration for memory and performance tracking
- **Optimization**: Queue family load balancing
- **Containers**: Additional specialized containers (circular buffers, static arrays)

### Under Consideration
- Vulkan Memory Allocator (VMA) integration
- Custom CPU memory allocators with Tracy integration
- Configuration file system for runtime settings

## Design Philosophy

ABox is designed around several key principles:

1. **Memory Safety**: RAII wrappers ensure proper resource cleanup and prevent leaks
2. **Stable Pointers**: Custom allocators (FetchList) provide stable memory addresses
3. **Version Tracking**: VersionedSlot enables safe resource reuse with ABA problem prevention
4. **Lock-Free Operations**: Futex-based synchronization for high-performance concurrent access
5. **Zero-Cost Abstractions**: C++20 features for performance without runtime overhead

## Documentation

- [CHANGELOG.md](CHANGELOG.md) - Detailed version history
- API Documentation: Coming soon

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Acknowledgments

- Built with Vulkan API
- Testing with Catch2
- Shader reflection using SPIRV-Reflect
