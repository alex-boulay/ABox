# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive VersionedSlot test suite with 17 test cases
- Cross-platform futex abstraction for better portability
- FetchList integration with VersionedSlot for versioned slot management
- FetchList iterator support with begin(), end(), cbegin(), cend() for range-based loops
- FetchList map-like API: at(), contains(), getHandleByIndex()
- Template-based MemoryWrapper tests for multiple pointer types (void*, MockHandle64*, MockHandle32*)
- MockHandleTraits pattern for type-safe test handle creation
- Handle struct for safe versioned element access
- DeviceHandler migration from std::list to FetchList with handle-based API
- Coverage reporting infrastructure with gcovr and HTML reports

### Changed
- Improved test coverage for VersionedSlot operations (edge cases for tryLock state transitions)
- Improved test coverage for FetchList operations (const at() exception paths)
- Enhanced FetchList with version tracking capabilities
- Reordered includes in VersionedSlot.hpp for consistency (alphabetical order)
- Improved code formatting in VersionedSlot (alignment, line breaks)
- DeviceHandler now uses FetchList handles instead of raw pointers for safer access
- Coverage configuration excludes Logger files and logging macros from metrics

### Removed
- GitHub Actions CI/CD workflow (maintenance overhead)

### Fixed
- Narrowing conversion warning in VersionedSlot::getDiagnostics()
- Unused variable warning in VersionedSlot tests
- Member initialization order in FetchList (multiplier_ before elements_per_block_)
- Futex wait logic in VersionedSlot::lock() - clarified fall-through behavior
- Critical futex deadlock in VersionedSlot unlock (wake all threads instead of one)

## [0.3.0] - 2026-01-11

### Added
- Catch2 testing infrastructure with CTest integration
- MemoryWrapper comprehensive test suite
- VersionedSlot utility for lock-free slot management with versioning
- FetchList colony-style allocator with stable pointers
- Cache-aligned block allocation
- BUILD_TESTS CMake option
- GitHub Actions CI/CD workflow (later removed)

### Changed
- Replaced `hardware_destructive_interference_size` with `ABOX_CACHE_LINE_SIZE`
- Project structure reorganization (moved app to separate folder)
- Improved FetchList design with bloom filter integration

## [0.2.0] - 2025-12-31

### Added
- Unified Logger system with category-based filtering
- SPIRV-Reflect integration for automatic descriptor set layout generation
- PipelineManager for unified management of Graphics, Compute, and RayTracing pipelines
- Variant-based heterogeneous pipeline storage
- C++20 ranges support for shader loading
- PipelineBase class with reflection-based layout creation
- ComputePipeline implementation
- RayTracingPipeline draft
- Per-frame debug activation/deactivation
- Pipeline naming/retrieval system

### Changed
- Migrated from scattered std::cout to unified Logger
- Refactored pipelines to inherit from PipelineBase
- Enhanced SPIRV-Reflect integration
- Improved PipelineManager with better pipeline management
- Updated clang-format configuration
- Better memory tracking logs

### Fixed
- SwapchainManager format bug
- Swapchain resize issues
- Renderpass attribute in resize operations

## [0.1.0] - 2025-06-XX

### Added
- MemoryWrapper RAII system for automatic Vulkan resource management
- DeviceHandler and DeviceBoundElements architecture
- SwapchainManager with recreation support
- FrameBufferBroker for framebuffer management
- CommandsHandler for command buffer management
- QueueManager for queue family handling
- SynchronisationManager with fences and semaphores
- ShaderHandler for shader module loading and SPIR-V compilation
- GraphicsPipeline with shader stage management
- RenderPassManager
- WindowManager with GLFW integration
- ResourcesManager for centralized Vulkan resource management
- Debug handler with validation layers
- Window resize callbacks
- Automatic shader compilation from GLSL to SPIR-V
- CMAKE build system with Ninja support
- clang-format integration
- Per-device and per-instance extension handling

### Changed
- Separated library from executable in CMake
- Made SHADER_DIR executable-only to decouple library from shader paths
- Improved queue family selection and management
- Enhanced debug logging throughout the system
- Refactored device ownership patterns

### Fixed
- GLFW destruction issues
- Surface creation and management
- Memory leaks in resource management
- Command buffer frame mapping
- Queue family indices mapping
- Swapchain recreation timing
- Image view creation
- Validation layer availability handling

## [0.0.1] - 2024-12-XX (Initial Development)

### Added
- Initial Vulkan instance creation
- Physical device enumeration and selection
- Logical device creation
- Validation layers support
- Surface creation with GLFW
- Swapchain setup
- Image views
- Queue family management
- Basic project structure
- Makefile (later replaced with CMake)

### Changed
- Cleaned up static variables to anonymous namespaces
- Improved shader handler data structure
- Enhanced extension reading

### Fixed
- VkCreateInstance validation issues
- Queue family detection
- Surface compatibility checks
- Swapchain support queries

---

## Version History Summary

- **v0.3.x**: Testing infrastructure and utility containers (FetchList, VersionedSlot)
- **v0.2.x**: Pipeline system unification, logging, and SPIRV-Reflect integration
- **v0.1.x**: Core Vulkan infrastructure, resource management, rendering pipeline
- **v0.0.x**: Initial Vulkan setup and basic functionality

## Notes

This project is a Vulkan-based rendering engine with a focus on:
- Memory safety through RAII wrappers
- Lock-free data structures
- Custom allocators for performance
- Automatic resource management
- Flexible pipeline management
