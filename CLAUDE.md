# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

MoonRender is a Windows-only C++17 Direct3D project. It requires Visual Studio 2022 Build Tools (v145), Windows SDK, and CMake 3.21+.

**Preferred: CMake with presets**

```powershell
cmake --preset vs-debug-x64
cmake --build --preset build-vs-debug-x64
```

Or use the wrapper scripts in `Scripts/`:
- `Scripts\Build_CMake_VS_Debug_x64.bat`
- `Scripts\Build_MSBuild_Debug_x64.bat` (legacy solution)

**Outputs**
- CMake binaries: `Builds/CMakeOutput/Bin/Debug/MoonRender.exe`
- MSBuild binaries: `Builds/MSBuild/x64/Debug/`
- Shader cache: `Builds/Cache/CSO/`
- ImGui layout state: `Builds/Runtime/imgui.ini`

**Clean shader cache**
```powershell
Scripts\Clean_ShaderCache.bat
```

## Architecture

### Application Flow

Entry point is `Source/AppWin/private/MainEntry.cpp` (`WinMain`). It shows a startup dialog to choose the graphics backend (DX11 or DX12), then instantiates `App` and calls `Run()`.

`D3DApp` (`Source/AppWin/public/D3DApp.h`) is the base framework:
- Creates the Win32 window and message pump
- Initializes the graphics backend and Dear ImGui
- Runs the frame loop: `UpdateScene(dt)` → `DrawScene()` → `DrawUI()`

`App` derives from `D3DApp` and owns the actual renderer:
- Maintains `m_sceneObjects` (vector of `MoonObject`)
- Creates shaders, buffers, and rasterizer state through the `IGraphicsBackend` abstraction
- Finds special objects by component type at frame time (main camera, directional light, first renderable)
- `DrawScene()` binds the backend, updates constant buffers, and issues `DrawIndexed`
- `DrawUI()` delegates to `MoonUI::UserInterface`

### Graphics Backend Abstraction

`IGraphicsBackend` (`Source/Graphics/public/GraphicsBackend.h`) wraps D3D resources behind a common interface. Concrete implementations live in `Source/Graphics/private/Dx11Backend.cpp` and `Dx12Backend.cpp`. The app selects the backend at startup via `GraphicsBackendFactory`.

Key abstraction types:
- `IGraphicsBuffer`, `IGraphicsVertexShader`, `IGraphicsPixelShader`
- `IGraphicsInputLayout`, `IGraphicsRasterizerState`
- `GraphicsBufferDesc`, `GraphicsShaderDesc`, `GraphicsRasterizerDesc`

The backend also owns ImGui platform/renderer initialization and per-frame rendering.

### Object / Component System

`MoonObject` (`Source/Object/MoonObject.h`) is a lightweight entity that holds a list of `MoonComponent`s.

- `AddComponent<T>()`, `GetComponent<T>()`, `HasComponent<T>()` — template helpers with `dynamic_cast`
- Components can declare conflict groups; only one component per conflict group is allowed on an object (e.g., only one `CameraComponent`)
- `MOON_COMPONENT(TypeName, DisplayName, ConflictGroup)` macro is required on every concrete component

`MoonComponent` (`Source/Object/MoonComponent.h`) exposes a property reflection system via `ComponentProperty`. Components register properties in their constructor using helpers in the `MoonProp` namespace (`Float3`, `Float`, `Float4`, `Text`). Properties carry getter/setter lambdas and ImGui metadata (speed, format, clamp range). The `UserInterface` inspector reads these properties generically to render editable controls without component-specific UI code.

Built-in components:
- `TransformComponent` — position, rotation, scale
- `MeshComponent` — references a loaded `Mesh`
- `CameraComponent` — view/projection settings
- `LightComponent` — color, intensity, light type
- `AutoRotateComponent` — continuous rotation

### UI

`MoonUI::UserInterface` (`Source/UI/UserInterface.cpp`) builds the Dear ImGui interface:
- **Outline** — scene hierarchy, click to select an object
- **Inspector** — edits the selected object's component properties via the reflection system
- **Global Settings** — backend info and engine settings
- **Output Log** — captures logs from `Logging::LogSystem`

### Events and Logging

`EventCenter` (`Source/EventSystem/EventCenter.h`) is a string-keyed singleton callback system (`std::function`).

`LogSystem` (`Source/Logging/public/LogSystem.h`) buffers log entries that the UI output window consumes each frame.

### Asset Loading

`MoonMeshLoader` uses `tinyobjloader` to load `.obj` files into `Mesh` structures at runtime. Shaders are raw `.hlsl` files compiled at runtime through the backend's `CompileShader` method. Paths are resolved from the repository root via the `MOONRENDER_PROJECT_ROOT` CMake definition.

## Code Style

- `public/` and `private/` subdirectories separate headers from implementation within each module
- Includes are often fully qualified from the repository root (e.g., `#include "Source/Object/MoonComponent.h"`)
- `WinMin.h` provides lean Windows.h includes; use it instead of including `<windows.h>` directly
- `UNICODE` and `_UNICODE` are defined globally
