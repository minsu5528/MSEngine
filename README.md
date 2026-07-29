# MSEngine Development

This project is a successor to [TetrisCore_Engine](https://github.com/minsu5528/TetrisCore_Engine), built to deepen control over low-level systems — memory, math, and rendering — without relying on a commercial engine. MSEngine inherits the validated core systems from TetrisCore and extends them into a DirectX 11 based rendering pipeline.

## Predecessor Project

[**TetrisCore_Engine**](https://github.com/minsu5528/TetrisCore_Engine) — a C++ testbed for memory tracking and object pooling, built without any commercial engine. Core systems (Memory Tracker, Object Pool, Math Library) were validated here before being migrated into MSEngine.

## Version Roadmap

### Core Systems Migration (Completed)

Inherited and modernized the core systems proven in TetrisCore_Engine.

- Global `new`/`delete`/`new[]`/`delete[]` overloading for runtime memory leak detection
- Fixed-size Object Pool for zero dynamic allocation at runtime
- Vector2 / Matrix2x2 math library
- Migrated fixed-size containers from raw C arrays to `std::array` for safer bounds-checked access

### v0.7 - dx11-bootstrap (Completed)

- Win32 window creation
- Direct3D 11 device and swap chain initialization
- First triangle render
- MemoryTracker extended to track DX11 resource lifetimes by category (Buffer / Shader / Texture) with peak GPU memory tracking
- Cross-validated against the debug layer's `ID3D11Debug::ReportLiveDeviceObjects`

```
[Resource Report]
Buffers created: 1, destroyed: 1
Shaders created: 2, destroyed: 2
Textures created: 0, destroyed: 0
Peak GPU memory: 1036 bytes
Outstanding: 0
```

### v0.8 - 3d-transform-camera (Completed)

- Index Buffer based cube rendering (8 vertices, 36 indices, no duplicate vertex data)
- Transform system (position / rotation / scale) using DirectXMath, applied in Scale → Rotation → Translation order
- Constant Buffer for per-frame World / View / Projection matrix upload (Map/Unmap)
- Depth-Stencil buffer for correct face occlusion
- Camera class with WASD movement, DeltaTime-based frame-rate-independent motion
- Non-blocking render loop (PeekMessage) enabling continuous per-frame rendering and rotation
- Multiple objects (3 cubes) sharing a single model via an array of Transforms
- Verified with RenderDoc frame capture — debug-named resources (via `SetPrivateData`) visible in the Pipeline State view

### Scene Architecture (Planned, Fall Semester)

- Scene / GameObject / Component hierarchy
- Job Queue and Thread Pool introduction (multithreading basics)

### v1.0 - culling-instancing (Planned, Winter Break)

- Frustum Culling implementation
- GPU Instancing for large-scale object rendering
- Performance profiling via RenderDoc (Draw Call count, Frame Time, before/after comparison)

### v1.1 - portfolio-demo (Planned)

- Final README with performance benchmarks
- 2-3 minute demo video

## Tech Stack

- **Language**: C++ (Modern C++11/14)
- **Graphics API**: DirectX 11
- **Platform**: Windows (Win32)
- **Tools**: Visual Studio, RenderDoc

## Project Structure

```
MSEngine/
  Core/
    Memory/    # MemoryTracker, ObjectPool, ResourceTracker
    Math/
      Transform.h
      Legacy/  # Vector2, Matrix2x2 — inherited from TetrisCore (2D), unused after the 3D transition
    Time/      # Timer (DeltaTime)
  Renderer/
    Camera.h / Camera.cpp
    Shaders/   # VertexShader.hlsl, PixelShader.hlsl
  main.cpp
```
