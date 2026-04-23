#include "../public/BufferStruct.h"

#include <iterator>

namespace
{
    constexpr VertexAttributeDesc kVertexPosColorLayout[] = {
        { "POSITION", 0, GraphicsFormat::R32G32B32_FLOAT, 0 },
        { "COLOR", 0, GraphicsFormat::R32G32B32A32_FLOAT, 12 }
    };

    constexpr VertexAttributeDesc kVertexPosNormalColorLayout[] = {
        { "POSITION", 0, GraphicsFormat::R32G32B32_FLOAT, 0 },
        { "NORMAL", 0, GraphicsFormat::R32G32B32_FLOAT, 12 },
        { "COLOR", 0, GraphicsFormat::R32G32B32A32_FLOAT, 24 }
    };

    constexpr VertexAttributeDesc kVertexPosNormalLayout[] = {
        { "POSITION", 0, GraphicsFormat::R32G32B32_FLOAT, 0 },
        { "NORMAL", 0, GraphicsFormat::R32G32B32_FLOAT, 12 }
    };

    constexpr VertexAttributeDesc kVertexPosNormalColorUvLayout[] = {
        { "POSITION", 0, GraphicsFormat::R32G32B32_FLOAT, 0 },
        { "NORMAL", 0, GraphicsFormat::R32G32B32_FLOAT, 12 },
        { "COLOR", 0, GraphicsFormat::R32G32B32A32_FLOAT, 24 },
        { "TEXCOORD", 0, GraphicsFormat::R32G32_FLOAT, 40 }
    };
}

VertexLayoutDesc BufferStruct::VertexPosColor::GetVertexLayout()
{
    return { kVertexPosColorLayout, static_cast<std::uint32_t>(std::size(kVertexPosColorLayout)) };
}

VertexLayoutDesc BufferStruct::VertexPosNormalColor::GetVertexLayout()
{
    return { kVertexPosNormalColorLayout, static_cast<std::uint32_t>(std::size(kVertexPosNormalColorLayout)) };
}

VertexLayoutDesc BufferStruct::VertexPosNormal::GetVertexLayout()
{
    return { kVertexPosNormalLayout, static_cast<std::uint32_t>(std::size(kVertexPosNormalLayout)) };
}

VertexLayoutDesc BufferStruct::VertexPosNormalColorUV::GetVertexLayout()
{
    return { kVertexPosNormalColorUvLayout, static_cast<std::uint32_t>(std::size(kVertexPosNormalColorUvLayout)) };
}
