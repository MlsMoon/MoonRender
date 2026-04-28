# MoonRender

MoonRender is a Windows Direct3D 11 sample project with Dear ImGui integration.

## Requirements

- Windows 10 or newer
- Visual Studio Build Tools with MSVC and Windows SDK
- CMake 3.21+ for the CMake workflow

## Repository Layout

- `Source/` and `Resources/`: source code and runtime assets
- `Scripts/`: shortcut batch files for common tasks
- `Builds/`: unified location for build outputs, caches, and generated CMake build trees

## Build

### Rider / MSBuild / Solution

The legacy solution is still supported for Rider and other MSBuild-based workflows.

```powershell
Scripts\Build_MSBuild_Debug_x64.bat
```

MSBuild outputs are written to:

```text
Builds/MSBuild/x64/Debug/
Builds/Intermediate/MSBuild/x64/Debug/
```

### CMake

The repository includes `CMakePresets.json` so CMake build trees also stay under `Builds/`.

```powershell
Scripts\build_debug.bat
Scripts\build_release.bat
```

The direct command used during development is:

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build --preset build-vs-debug-x64
```

If the build tree has not been generated yet, configure it first:

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --preset vs-debug-x64
```

CMake outputs are written under:

```text
Builds/CMake/
Builds/CMakeOutput/
```

## Utility Scripts

- `Scripts\build_debug.bat` / `Scripts\build_release.bat`: build via CMake Visual Studio preset
- `Scripts\run_debug.bat` / `Scripts\run_release.bat`: launch the executable
- `Scripts\renderdoc_debug.bat` / `Scripts\renderdoc_release.bat`: launch through RenderDoc
- `Scripts\clean_shaders.bat`: remove generated shader cache from `Builds/Cache/CSO`
- `Scripts\clean_legacy.bat`: remove old root-level build folders after closing Rider/MSBuild

## Notes

- Dear ImGui layout state is now stored in `Builds/Runtime/imgui.ini`
- Shader cache is now stored in `Builds/Cache/CSO`
- Runtime assets are still loaded from the repository root via absolute project-root-based paths

## References

- [MKXJun/DirectX11-With-Windows-SDK](https://github.com/MKXJun/DirectX11-With-Windows-SDK)
- [microsoft/DirectX-Graphics-Samples](https://github.com/microsoft/DirectX-Graphics-Samples)
- [ocornut/imgui example_win32_directx12](https://github.com/ocornut/imgui/tree/master/examples/example_win32_directx12)
- [d3dcoder/d3d12book](https://github.com/d3dcoder/d3d12book)
- *Introduction to 3D Game Programming with DirectX 11*
