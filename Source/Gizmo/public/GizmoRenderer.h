#pragma once

#include <cstdint>
#include <memory>

#include "Source/Graphics/public/GraphicsBackend.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"
#include "Source/Math/public/MoonMath.h"

enum class GizmoAxis
{
    None,
    X,
    Y,
    Z
};

class GizmoRenderer
{
public:
    bool Init(IGraphicsBackend& graphics);
    void Render(IGraphicsBackend& graphics, const BufferStruct::ConstantMVPBuffer& mvp, GizmoAxis hoveredAxis);

    GizmoAxis Raycast(const MoonVector3& rayOrigin, const MoonVector3& rayDir, const MoonMatrix4x4& gizmoWorldMatrix);

    static MoonVector3 GetAxisDirection(GizmoAxis axis);

private:
    void UpdateVertexColors(IGraphicsBackend& graphics, GizmoAxis hoveredAxis);

private:
    std::shared_ptr<IGraphicsBuffer> m_VertexBuffer;
    std::shared_ptr<IGraphicsBuffer> m_IndexBuffer;
    std::shared_ptr<IGraphicsBuffer> m_ConstantBuffer;
    std::shared_ptr<IGraphicsVertexShader> m_VertexShader;
    std::shared_ptr<IGraphicsPixelShader> m_PixelShader;
    std::shared_ptr<IGraphicsInputLayout> m_InputLayout;
    std::uint32_t m_IndexCount = 0;
};
