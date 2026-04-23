# MoonRender

MoonRender is a Windows Direct3D 11 sample project with Dear ImGui integration.

## Requirements

- Windows 10 or newer
- CMake 3.21+
- MSVC toolchain from Visual Studio 2022 Build Tools or Visual Studio 2022
- Windows SDK with Direct3D 11 headers and libraries

This repository is now built with CMake. Visual Studio solution files are kept only as legacy project files.

## Build

### Ninja + MSVC

Open a Developer Command Prompt for VS 2022 or any shell where the MSVC environment is already loaded, then run:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Visual Studio Generator

```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64
cmake --build build-vs --config Debug
```

## Run

The executable loads fonts, models, shaders, and shader cache paths from the repository root, so it can be launched from either generator layout without copying `Resources`.

## References

- [MKXJun/DirectX11-With-Windows-SDK](https://github.com/MKXJun/DirectX11-With-Windows-SDK)
- *Introduction to 3D Game Programming with DirectX 11*
