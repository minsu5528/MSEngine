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

### v0.7 - dx11-bootstrap (In Progress)

- Win32 window creation
- Direct3D 11 device and swap chain initialization
- First triangle render
- Zero memory leak verification via Memory Tracker

### v0.8 - 3d-transform-camera (Planned)

- 3D cube rendering
- Transform system (position / rotation / scale)
- Camera movement (view / projection matrices)

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
    Memory/    # MemoryTracker, ObjectPool
    Math/      # Vector2, Matrix2x2
  Renderer/    # DirectX 11 rendering systems (in progress)
  main.cpp
```
