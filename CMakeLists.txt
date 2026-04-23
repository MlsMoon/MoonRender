cmake_minimum_required(VERSION 3.21)

project(MoonRender LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "MoonRender is a Windows-only project.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "MoonRender currently supports x64 builds only.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "MoonRender requires the MSVC toolchain.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MOONRENDER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(MOONRENDER_OUTPUT_ROOT "${MOONRENDER_ROOT}/Builds/CMakeOutput")

set(MOONRENDER_IMGUI_SOURCES
    Source/ThirdParty/ImGui/imgui.cpp
    Source/ThirdParty/ImGui/imgui_demo.cpp
    Source/ThirdParty/ImGui/imgui_draw.cpp
    Source/ThirdParty/ImGui/imgui_impl_dx11.cpp
    Source/ThirdParty/ImGui/imgui_impl_win32.cpp
    Source/ThirdParty/ImGui/imgui_tables.cpp
    Source/ThirdParty/ImGui/imgui_widgets.cpp
)

set(MOONRENDER_SOURCES
    Source/EventSystem/EventCenter.cpp
    Source/GameObject/Camera.cpp
    Source/Logging/private/LogSystem.cpp
    Source/ResourcesProcess/private/Mesh.cpp
    Source/ResourcesProcess/private/MoonMeshLoader.cpp
    Source/ThirdParty/tinyobjloader/tiny_obj_loader.cpp
    Source/UI/UserInterface.cpp
    Source/AppWin/private/CpuTimer.cpp
    Source/AppWin/private/D3DApp.cpp
    Source/AppWin/private/D3DUtil.cpp
    Source/AppWin/private/DXTrace.cpp
    Source/AppWin/private/GameApp.cpp
    Source/AppWin/private/Main.cpp
    Source/ResourcesProcess/private/BufferStruct.cpp
)

set(MOONRENDER_HEADERS
    Source/AppWin/public/MoonRenderClass.h
    Source/EventSystem/EventCenter.h
    Source/GameObject/Camera.h
    Source/Logging/public/LogSystem.h
    Source/Render/LightType.h
    Source/Render/MaterialParam.h
    Source/ResourcesProcess/public/Mesh.h
    Source/ResourcesProcess/public/MoonMeshLoader.h
    Source/ThirdParty/tinyobjloader/tiny_obj_loader.h
    Source/UI/UserInterface.h
    Source/AppWin/public/CpuTimer.h
    Source/AppWin/public/D3DApp.h
    Source/AppWin/public/D3DUtil.h
    Source/AppWin/public/DXTrace.h
    Source/AppWin/public/GameApp.h
    Source/AppWin/public/WinMin.h
    Source/ResourcesProcess/public/BufferStruct.h
    Source/ThirdParty/ImGui/imconfig.h
    Source/ThirdParty/ImGui/imgui.h
    Source/ThirdParty/ImGui/imgui_impl_dx11.h
    Source/ThirdParty/ImGui/imgui_impl_win32.h
    Source/ThirdParty/ImGui/imgui_internal.h
    Source/ThirdParty/ImGui/imstb_rectpack.h
    Source/ThirdParty/ImGui/imstb_textedit.h
    Source/ThirdParty/ImGui/imstb_truetype.h
)

add_library(imgui STATIC ${MOONRENDER_IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    "${MOONRENDER_ROOT}"
    "${MOONRENDER_ROOT}/Source/ThirdParty/ImGui"
)
target_compile_definitions(imgui PRIVATE UNICODE _UNICODE)

add_executable(MoonRender WIN32
    ${MOONRENDER_SOURCES}
    ${MOONRENDER_HEADERS}
)

target_include_directories(MoonRender PRIVATE "${MOONRENDER_ROOT}")
target_compile_definitions(MoonRender PRIVATE
    UNICODE
    _UNICODE
    MOONRENDER_PROJECT_ROOT="${MOONRENDER_ROOT}"
)

target_link_libraries(MoonRender PRIVATE
    imgui
    d3d11
    dxgi
    dxguid
    d3dcompiler
    winmm
    imm32
    gdi32
    dwmapi
)

target_compile_options(imgui PRIVATE /W3 /permissive-)
target_compile_options(MoonRender PRIVATE /W3 /permissive-)
target_link_options(MoonRender PRIVATE /SUBSYSTEM:WINDOWS)

set_target_properties(MoonRender PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${MOONRENDER_ROOT}"
)

foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(imgui PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${MOONRENDER_OUTPUT_ROOT}/Lib/${config}"
        PDB_OUTPUT_DIRECTORY_${config_upper} "${MOONRENDER_OUTPUT_ROOT}/Lib/${config}"
    )
    set_target_properties(MoonRender PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${MOONRENDER_OUTPUT_ROOT}/Bin/${config}"
        ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${MOONRENDER_OUTPUT_ROOT}/Lib/${config}"
        PDB_OUTPUT_DIRECTORY_${config_upper} "${MOONRENDER_OUTPUT_ROOT}/Bin/${config}"
    )
endforeach()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MoonRender)
