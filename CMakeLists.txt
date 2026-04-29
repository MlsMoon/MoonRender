cmake_minimum_required(VERSION 3.21)

if(APPLE)
    project(MoonRender LANGUAGES CXX OBJCXX)
else()
    project(MoonRender LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MOONRENDER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(MOONRENDER_OUTPUT_ROOT "${MOONRENDER_ROOT}/Builds/CMakeOutput")

# FetchContent dependencies
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm glfw nlohmann_json)

set(MOONRENDER_IMGUI_SOURCES
    Source/ThirdParty/ImGui/imgui.cpp
    Source/ThirdParty/ImGui/imgui_demo.cpp
    Source/ThirdParty/ImGui/imgui_draw.cpp
    Source/ThirdParty/ImGui/imgui_tables.cpp
    Source/ThirdParty/ImGui/imgui_widgets.cpp
)

if(WIN32)
    list(APPEND MOONRENDER_IMGUI_SOURCES
        Source/ThirdParty/ImGui/imgui_impl_glfw.cpp
        Source/ThirdParty/ImGui/imgui_impl_dx11.cpp
    )
elseif(APPLE)
    list(APPEND MOONRENDER_IMGUI_SOURCES
        Source/ThirdParty/ImGui/imgui_impl_glfw.cpp
        Source/ThirdParty/ImGui/imgui_impl_metal.mm
    )
endif()

set(MOONRENDER_SOURCES
    Source/EventSystem/EventCenter.cpp
    Source/Object/MoonObject.cpp
    Source/Object/Scene.cpp
    Source/Object/MeshComponent.cpp
    Source/Graphics/private/GraphicsBackendFactory.cpp
    Source/Logging/private/LogSystem.cpp
    Source/Math/private/MoonMath.cpp
    Source/ResourcesProcess/private/Mesh.cpp
    Source/ResourcesProcess/private/MoonMeshLoader.cpp
    Source/ThirdParty/tinyobjloader/tiny_obj_loader.cpp
    Source/UI/Editor.cpp
    Source/UI/MainMenuBar.cpp
    Source/UI/OutlineWindow.cpp
    Source/UI/InspectorWindow.cpp
    Source/UI/GlobalSettingWindow.cpp
    Source/UI/OutputLogWindow.cpp
    Source/UI/ViewportWindow.cpp
    Source/UI/Popups/PopupBase.cpp
    Source/UI/Popups/DeleteCameraWarningPopup.cpp
    Source/AppWin/private/CpuTimer.cpp
    Source/AppWin/private/D3DApp.cpp
    Source/AppWin/private/D3DUtil.cpp
    Source/AppWin/private/DXTrace.cpp
    Source/AppWin/private/App.cpp
    Source/AppWin/private/MainEntry.cpp
    Source/ResourcesProcess/private/BufferStruct.cpp
    Source/Gizmo/private/GizmoRenderer.cpp
    Source/Gizmo/private/GridRenderer.cpp
    Source/AssetSystem/private/MoonAssetFormat.cpp
    Source/AssetSystem/private/MoonAssetRegistry.cpp
    Source/AssetSystem/private/MoonAssetManager.cpp
    Source/AssetSystem/private/MoonAssetDependencyGraph.cpp
    Source/AssetSystem/private/MoonMeshAsset.cpp
    Source/AssetSystem/private/MoonAssetImporter.cpp
)

if(WIN32)
    list(APPEND MOONRENDER_SOURCES
        Source/Graphics/private/Dx11Backend.cpp
        Source/Graphics/private/Dx12Backend.cpp
    )
elseif(APPLE)
    list(APPEND MOONRENDER_SOURCES
        Source/Graphics/private/MetalBackend.mm
    )
endif()

set(MOONRENDER_HEADERS
    Source/AppWin/public/MoonRenderClass.h
    Source/EventSystem/EventCenter.h
    Source/Object/AutoRotateComponent.h
    Source/Object/CameraComponent.h
    Source/Object/LightComponent.h
    Source/Logging/public/LogSystem.h
    Source/Object/LightType.h
    Source/Object/MaterialParam.h
    Source/Object/MeshComponent.h
    Source/Object/MoonComponent.h
    Source/Object/MoonObject.h
    Source/Object/Scene.h
    Source/Object/TransformComponent.h
    Source/ResourcesProcess/public/Mesh.h
    Source/ResourcesProcess/public/MoonMeshLoader.h
    Source/ThirdParty/tinyobjloader/tiny_obj_loader.h
    Source/UI/Editor.h
    Source/UI/MainMenuBar.h
    Source/UI/OutlineWindow.h
    Source/UI/InspectorWindow.h
    Source/UI/GlobalSettingWindow.h
    Source/UI/OutputLogWindow.h
    Source/UI/ViewportWindow.h
    Source/AppWin/public/CpuTimer.h
    Source/AppWin/public/D3DApp.h
    Source/AppWin/public/D3DUtil.h
    Source/AppWin/public/DXTrace.h
    Source/AppWin/public/App.h
    Source/Math/public/MoonMath.h
    Source/Graphics/public/GraphicsBackend.h
    Source/Graphics/public/GraphicsBackendFactory.h
    Source/Graphics/public/GraphicsTypes.h
    Source/ResourcesProcess/public/BufferStruct.h
    Source/AssetSystem/public/MoonAssetFormat.h
    Source/AssetSystem/public/MoonAssetHandle.h
    Source/AssetSystem/public/MoonAssetRegistry.h
    Source/AssetSystem/public/MoonAssetManager.h
    Source/AssetSystem/public/MoonAssetDependencyGraph.h
    Source/AssetSystem/public/MoonAssetImporter.h
    Source/AssetSystem/public/MoonMeshAsset.h
    Source/ThirdParty/ImGui/imconfig.h
    Source/ThirdParty/ImGui/imgui.h
    Source/ThirdParty/ImGui/imgui_internal.h
    Source/ThirdParty/ImGui/imstb_rectpack.h
    Source/ThirdParty/ImGui/imstb_textedit.h
    Source/ThirdParty/ImGui/imstb_truetype.h
    Source/UI/Popups/PopupBase.h
    Source/UI/Popups/DeleteCameraWarningPopup.h
)

if(WIN32)
    list(APPEND MOONRENDER_HEADERS
        Source/ThirdParty/ImGui/imgui_impl_dx11.h
        Source/ThirdParty/ImGui/imgui_impl_glfw.h
        Source/AppWin/public/WinMin.h
    )
elseif(APPLE)
    list(APPEND MOONRENDER_HEADERS
        Source/ThirdParty/ImGui/imgui_impl_glfw.h
        Source/ThirdParty/ImGui/imgui_impl_metal.h
    )
endif()

add_library(imgui STATIC ${MOONRENDER_IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    "${MOONRENDER_ROOT}"
    "${MOONRENDER_ROOT}/Source/ThirdParty/ImGui"
)

if(TARGET glfw)
    target_link_libraries(imgui PUBLIC glfw)
endif()

if(WIN32)
    target_compile_definitions(imgui PRIVATE UNICODE _UNICODE)
endif()

if(WIN32)
    add_executable(MoonRender WIN32
        ${MOONRENDER_SOURCES}
        ${MOONRENDER_HEADERS}
    )
else()
    add_executable(MoonRender
        ${MOONRENDER_SOURCES}
        ${MOONRENDER_HEADERS}
    )
endif()

target_include_directories(MoonRender PRIVATE "${MOONRENDER_ROOT}")

target_compile_definitions(MoonRender PRIVATE
    MOONRENDER_PROJECT_ROOT="${MOONRENDER_ROOT}"
)

if(WIN32)
    target_compile_definitions(MoonRender PRIVATE UNICODE _UNICODE)
endif()

target_link_libraries(MoonRender PRIVATE imgui glm::glm glfw nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(MoonRender PRIVATE
        d3d11
        d3d12
        dxgi
        dxguid
        d3dcompiler
        winmm
        imm32
        gdi32
        dwmapi
    )
elseif(APPLE)
    target_link_libraries(MoonRender PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework QuartzCore"
        "-framework IOKit"
        "-framework CoreFoundation"
    )
endif()

if(WIN32)
    target_compile_options(imgui PRIVATE /W3 /permissive-)
    target_compile_options(MoonRender PRIVATE /W3 /permissive-)
    target_link_options(MoonRender PRIVATE /SUBSYSTEM:WINDOWS)
endif()

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
