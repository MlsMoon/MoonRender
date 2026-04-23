#pragma once

#include <cstdint>
#include <string>

enum class GraphicsBackendType
{
    DX11,
    DX12
};

enum class GraphicsFormat
{
    Unknown,
    R32G32_FLOAT,
    R32G32B32_FLOAT,
    R32G32B32A32_FLOAT,
    R8G8B8A8_UNORM
};

enum class GraphicsBufferUsage
{
    Immutable,
    Dynamic
};

enum class GraphicsPrimitiveTopology
{
    TriangleList
};

enum class GraphicsIndexFormat
{
    UInt16,
    UInt32
};

enum class GraphicsFillMode
{
    Solid,
    Wireframe
};

enum class GraphicsCullMode
{
    None,
    Front,
    Back
};

enum class GraphicsShaderStage
{
    Vertex,
    Pixel,
    Compute
};

enum class GraphicsBufferBindFlags : std::uint32_t
{
    None = 0,
    VertexBuffer = 1 << 0,
    IndexBuffer = 1 << 1,
    ConstantBuffer = 1 << 2
};

inline GraphicsBufferBindFlags operator|(GraphicsBufferBindFlags lhs, GraphicsBufferBindFlags rhs)
{
    return static_cast<GraphicsBufferBindFlags>(
        static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

inline bool HasAnyFlag(GraphicsBufferBindFlags value, GraphicsBufferBindFlags flag)
{
    return (static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(flag)) != 0;
}

struct GraphicsViewport
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};

struct GraphicsBufferDesc
{
    std::uint32_t byteWidth = 0;
    GraphicsBufferUsage usage = GraphicsBufferUsage::Immutable;
    GraphicsBufferBindFlags bindFlags = GraphicsBufferBindFlags::None;
    bool cpuWrite = false;
    std::string debugName;
};

struct GraphicsShaderDesc
{
    std::wstring filePath;
    GraphicsShaderStage stage = GraphicsShaderStage::Vertex;
    std::string debugName;
};

struct GraphicsRasterizerDesc
{
    GraphicsFillMode fillMode = GraphicsFillMode::Solid;
    GraphicsCullMode cullMode = GraphicsCullMode::Back;
    bool frontCounterClockwise = false;
    bool depthClipEnable = true;
    std::string debugName;
};

struct VertexAttributeDesc
{
    const char* semanticName = "";
    std::uint32_t semanticIndex = 0;
    GraphicsFormat format = GraphicsFormat::Unknown;
    std::uint32_t alignedByteOffset = 0;
};

struct VertexLayoutDesc
{
    const VertexAttributeDesc* attributes = nullptr;
    std::uint32_t attributeCount = 0;
};
