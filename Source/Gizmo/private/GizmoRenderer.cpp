#include "Source/Gizmo/public/GizmoRenderer.h"

#include <vector>

#include "Source/AppWin/public/D3DUtil.h"
#include "Source/Logging/public/LogSystem.h"

namespace
{
    constexpr float kGizmoArrowLength = 1.0f;
    constexpr float kGizmoArrowRadius = 0.06f;
    constexpr float kGizmoRaycastThreshold = 0.08f;

    void AddTetrahedron(std::vector<BufferStruct::VertexPosColor>& vertices,
                        std::vector<std::uint32_t>& indices,
                        const MoonVector3& tip,
                        const MoonVector3& base0,
                        const MoonVector3& base1,
                        const MoonVector3& base2,
                        const MoonVector4& color)
    {
        std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
        vertices.push_back({ tip, color });
        vertices.push_back({ base0, color });
        vertices.push_back({ base1, color });
        vertices.push_back({ base2, color });

        // Side faces
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
        indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 1);
        // Base face
        indices.push_back(base + 1); indices.push_back(base + 3); indices.push_back(base + 2);
    }

    float RaySegmentDistance(const MoonVector3& rayOrigin, const MoonVector3& rayDir,
                             const MoonVector3& segStart, const MoonVector3& segEnd)
    {
        MoonVector3 segDir = segEnd - segStart;
        MoonVector3 diff = rayOrigin - segStart;

        float a = MoonDot(rayDir, rayDir);
        float b = MoonDot(rayDir, segDir);
        float c = MoonDot(segDir, segDir);
        float d = MoonDot(rayDir, diff);
        float e = MoonDot(segDir, diff);

        float denom = a * c - b * b;

        float s, t;
        if (denom < 1e-6f)
        {
            s = 0.0f;
            t = MoonClamp(e / c, 0.0f, 1.0f);
        }
        else
        {
            s = MoonClamp((b * e - c * d) / denom, 0.0f, 1.0f);
            t = (b * s + e) / c;
            if (t < 0.0f)
            {
                t = 0.0f;
                s = MoonClamp(-d / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = MoonClamp((b - d) / a, 0.0f, 1.0f);
            }
        }

        MoonVector3 closestOnRay = rayOrigin + rayDir * s;
        MoonVector3 closestOnSeg = segStart + segDir * t;
        return MoonLength(closestOnRay - closestOnSeg);
    }
}

bool GizmoRenderer::Init(IGraphicsBackend& graphics)
{
    std::vector<BufferStruct::VertexPosColor> vertices;
    std::vector<std::uint32_t> indices;

    // X axis: red
    AddTetrahedron(vertices, indices,
        MoonVector3(0.0f, 0.0f, 0.0f),
        MoonVector3(kGizmoArrowLength, kGizmoArrowRadius, kGizmoArrowRadius),
        MoonVector3(kGizmoArrowLength, -kGizmoArrowRadius, kGizmoArrowRadius),
        MoonVector3(kGizmoArrowLength, 0.0f, -kGizmoArrowRadius),
        MoonVector4(1.0f, 0.2f, 0.2f, 1.0f));

    // Y axis: green
    AddTetrahedron(vertices, indices,
        MoonVector3(0.0f, 0.0f, 0.0f),
        MoonVector3(kGizmoArrowRadius, kGizmoArrowLength, kGizmoArrowRadius),
        MoonVector3(-kGizmoArrowRadius, kGizmoArrowLength, kGizmoArrowRadius),
        MoonVector3(0.0f, kGizmoArrowLength, -kGizmoArrowRadius),
        MoonVector4(0.2f, 1.0f, 0.2f, 1.0f));

    // Z axis: blue
    AddTetrahedron(vertices, indices,
        MoonVector3(0.0f, 0.0f, 0.0f),
        MoonVector3(kGizmoArrowRadius, kGizmoArrowRadius, kGizmoArrowLength),
        MoonVector3(-kGizmoArrowRadius, kGizmoArrowRadius, kGizmoArrowLength),
        MoonVector3(0.0f, -kGizmoArrowRadius, kGizmoArrowLength),
        MoonVector4(0.2f, 0.4f, 1.0f, 1.0f));

    m_IndexCount = static_cast<std::uint32_t>(indices.size());

    GraphicsBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.byteWidth = static_cast<std::uint32_t>(vertices.size() * sizeof(BufferStruct::VertexPosColor));
    vertexBufferDesc.usage = GraphicsBufferUsage::Dynamic;
    vertexBufferDesc.bindFlags = GraphicsBufferBindFlags::VertexBuffer;
    vertexBufferDesc.cpuWrite = true;
    vertexBufferDesc.debugName = "GizmoVertexBuffer";
    m_VertexBuffer = graphics.CreateBuffer(vertexBufferDesc, vertices.data());

    GraphicsBufferDesc indexBufferDesc = {};
    indexBufferDesc.byteWidth = static_cast<std::uint32_t>(indices.size() * sizeof(std::uint32_t));
    indexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    indexBufferDesc.bindFlags = GraphicsBufferBindFlags::IndexBuffer;
    indexBufferDesc.debugName = "GizmoIndexBuffer";
    m_IndexBuffer = graphics.CreateBuffer(indexBufferDesc, indices.data());

    GraphicsBufferDesc constantBufferDesc = {};
    constantBufferDesc.usage = GraphicsBufferUsage::Dynamic;
    constantBufferDesc.bindFlags = GraphicsBufferBindFlags::ConstantBuffer;
    constantBufferDesc.cpuWrite = true;
    constantBufferDesc.byteWidth = sizeof(BufferStruct::ConstantMVPBuffer);
    constantBufferDesc.debugName = "GizmoMVPConstantBuffer";
    m_ConstantBuffer = graphics.CreateBuffer(constantBufferDesc, nullptr);

    GraphicsShaderDesc vsDesc = {};
    vsDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/Color_VS.hlsl");
    vsDesc.stage = GraphicsShaderStage::Vertex;
    vsDesc.entryPoint = "VS";
    vsDesc.debugName = "Color_VS";

    GraphicsShaderDesc psDesc = {};
    psDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/Color_PS.hlsl");
    psDesc.stage = GraphicsShaderStage::Pixel;
    psDesc.entryPoint = "PS";
    psDesc.debugName = "Color_PS";

    auto vsBytecode = graphics.CompileShader(vsDesc);
    auto psBytecode = graphics.CompileShader(psDesc);
    if (!vsBytecode || !psBytecode)
    {
        MOON_LOG("Failed to compile gizmo shaders");
        return false;
    }

    m_VertexShader = graphics.CreateVertexShader(*vsBytecode, "Color_VS");
    m_PixelShader = graphics.CreatePixelShader(*psBytecode, "Color_PS");
    m_InputLayout = graphics.CreateInputLayout(BufferStruct::VertexPosColor::GetVertexLayout(), *vsBytecode, "ColorInputLayout");

    return m_VertexBuffer && m_IndexBuffer && m_ConstantBuffer && m_VertexShader && m_PixelShader && m_InputLayout;
}

void GizmoRenderer::UpdateVertexColors(IGraphicsBackend& graphics, GizmoAxis hoveredAxis)
{
    struct ColorEntry
    {
        MoonVector4 normal;
        MoonVector4 highlight;
    };

    ColorEntry colors[3] = {
        { MoonVector4(1.0f, 0.2f, 0.2f, 1.0f), MoonVector4(1.0f, 0.6f, 0.6f, 1.0f) }, // X
        { MoonVector4(0.2f, 1.0f, 0.2f, 1.0f), MoonVector4(0.6f, 1.0f, 0.6f, 1.0f) }, // Y
        { MoonVector4(0.2f, 0.4f, 1.0f, 1.0f), MoonVector4(0.5f, 0.7f, 1.0f, 1.0f) }, // Z
    };

    std::vector<BufferStruct::VertexPosColor> vertices;
    vertices.reserve(12);

    for (int axis = 0; axis < 3; ++axis)
    {
        GizmoAxis gizmoAxis = static_cast<GizmoAxis>(axis + 1);
        const MoonVector4& color = (hoveredAxis == gizmoAxis) ? colors[axis].highlight : colors[axis].normal;

        // Each tetrahedron has 4 vertices
        for (int v = 0; v < 4; ++v)
        {
            // We need to reconstruct positions. For simplicity, regenerate them.
            // X axis
            if (axis == 0)
            {
                if (v == 0) vertices.push_back({ MoonVector3(0.0f, 0.0f, 0.0f), color });
                else if (v == 1) vertices.push_back({ MoonVector3(kGizmoArrowLength, kGizmoArrowRadius, kGizmoArrowRadius), color });
                else if (v == 2) vertices.push_back({ MoonVector3(kGizmoArrowLength, -kGizmoArrowRadius, kGizmoArrowRadius), color });
                else vertices.push_back({ MoonVector3(kGizmoArrowLength, 0.0f, -kGizmoArrowRadius), color });
            }
            // Y axis
            else if (axis == 1)
            {
                if (v == 0) vertices.push_back({ MoonVector3(0.0f, 0.0f, 0.0f), color });
                else if (v == 1) vertices.push_back({ MoonVector3(kGizmoArrowRadius, kGizmoArrowLength, kGizmoArrowRadius), color });
                else if (v == 2) vertices.push_back({ MoonVector3(-kGizmoArrowRadius, kGizmoArrowLength, kGizmoArrowRadius), color });
                else vertices.push_back({ MoonVector3(0.0f, kGizmoArrowLength, -kGizmoArrowRadius), color });
            }
            // Z axis
            else
            {
                if (v == 0) vertices.push_back({ MoonVector3(0.0f, 0.0f, 0.0f), color });
                else if (v == 1) vertices.push_back({ MoonVector3(kGizmoArrowRadius, kGizmoArrowRadius, kGizmoArrowLength), color });
                else if (v == 2) vertices.push_back({ MoonVector3(-kGizmoArrowRadius, kGizmoArrowRadius, kGizmoArrowLength), color });
                else vertices.push_back({ MoonVector3(0.0f, -kGizmoArrowRadius, kGizmoArrowLength), color });
            }
        }
    }

    graphics.UpdateBuffer(*m_VertexBuffer, vertices.data(), vertices.size() * sizeof(BufferStruct::VertexPosColor));
}

void GizmoRenderer::Render(IGraphicsBackend& graphics, const BufferStruct::ConstantMVPBuffer& mvp, GizmoAxis hoveredAxis)
{
    UpdateVertexColors(graphics, hoveredAxis);

    graphics.UpdateBuffer(*m_ConstantBuffer, &mvp, sizeof(mvp));

    graphics.SetVertexBuffer(*m_VertexBuffer, sizeof(BufferStruct::VertexPosColor), 0);
    graphics.SetIndexBuffer(*m_IndexBuffer, GraphicsIndexFormat::UInt32, 0);
    graphics.SetPrimitiveTopology(GraphicsPrimitiveTopology::TriangleList);
    graphics.SetInputLayout(m_InputLayout.get());
    graphics.SetVertexShader(m_VertexShader.get());
    graphics.SetPixelShader(m_PixelShader.get());
    graphics.SetVertexConstantBuffer(0, m_ConstantBuffer.get());
    graphics.SetPixelConstantBuffer(1, nullptr);

    graphics.DrawIndexed(m_IndexCount, 0, 0);
}

GizmoAxis GizmoRenderer::Raycast(const MoonVector3& rayOrigin, const MoonVector3& rayDir, const MoonMatrix4x4& gizmoWorldMatrix)
{
    // Transform ray to gizmo local space
    MoonMatrix4x4 invWorld = gizmoWorldMatrix.Inverse();
    MoonVector3 localOrigin = invWorld.TransformPosition(rayOrigin);
    MoonVector3 localDir = invWorld.TransformDirection(rayDir);
    localDir = MoonNormalize(localDir);

    float bestDist = kGizmoRaycastThreshold;
    GizmoAxis bestAxis = GizmoAxis::None;

    struct AxisSeg { MoonVector3 start; MoonVector3 end; GizmoAxis axis; };
    AxisSeg axes[3] = {
        { MoonVector3(0,0,0), MoonVector3(kGizmoArrowLength, 0, 0), GizmoAxis::X },
        { MoonVector3(0,0,0), MoonVector3(0, kGizmoArrowLength, 0), GizmoAxis::Y },
        { MoonVector3(0,0,0), MoonVector3(0, 0, kGizmoArrowLength), GizmoAxis::Z },
    };

    for (const auto& seg : axes)
    {
        float dist = RaySegmentDistance(localOrigin, localDir, seg.start, seg.end);
        if (dist < bestDist)
        {
            bestDist = dist;
            bestAxis = seg.axis;
        }
    }

    return bestAxis;
}

MoonVector3 GizmoRenderer::GetAxisDirection(GizmoAxis axis)
{
    switch (axis)
    {
    case GizmoAxis::X: return MoonVector3(1.0f, 0.0f, 0.0f);
    case GizmoAxis::Y: return MoonVector3(0.0f, 1.0f, 0.0f);
    case GizmoAxis::Z: return MoonVector3(0.0f, 0.0f, 1.0f);
    default: return MoonVector3(0.0f, 0.0f, 0.0f);
    }
}
