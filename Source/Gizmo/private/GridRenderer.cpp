#include "Source/Gizmo/public/GridRenderer.h"

#include "Source/AppWin/public/D3DUtil.h"
#include "Source/Logging/public/LogSystem.h"

bool GridRenderer::Init(IGraphicsBackend& graphics)
{
    // Grid plane: large quad on XZ plane at Y=0
    const float gridExtent = 50.0f;
    BufferStruct::VertexPosColor vertices[4] = {
        { MoonVector3(-gridExtent, 0.0f, -gridExtent), MoonVector4(1.0f, 1.0f, 1.0f, 1.0f) },
        { MoonVector3( gridExtent, 0.0f, -gridExtent), MoonVector4(1.0f, 1.0f, 1.0f, 1.0f) },
        { MoonVector3(-gridExtent, 0.0f,  gridExtent), MoonVector4(1.0f, 1.0f, 1.0f, 1.0f) },
        { MoonVector3( gridExtent, 0.0f,  gridExtent), MoonVector4(1.0f, 1.0f, 1.0f, 1.0f) },
    };

    const std::uint32_t indices[6] = { 0, 1, 2, 1, 3, 2 };
    m_IndexCount = 6;

    GraphicsBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.byteWidth = sizeof(vertices);
    vertexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    vertexBufferDesc.bindFlags = GraphicsBufferBindFlags::VertexBuffer;
    vertexBufferDesc.debugName = "GridVertexBuffer";
    m_VertexBuffer = graphics.CreateBuffer(vertexBufferDesc, vertices);

    GraphicsBufferDesc indexBufferDesc = {};
    indexBufferDesc.byteWidth = sizeof(indices);
    indexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    indexBufferDesc.bindFlags = GraphicsBufferBindFlags::IndexBuffer;
    indexBufferDesc.debugName = "GridIndexBuffer";
    m_IndexBuffer = graphics.CreateBuffer(indexBufferDesc, indices);

    GraphicsBufferDesc constantBufferDesc = {};
    constantBufferDesc.usage = GraphicsBufferUsage::Dynamic;
    constantBufferDesc.bindFlags = GraphicsBufferBindFlags::ConstantBuffer;
    constantBufferDesc.cpuWrite = true;
    constantBufferDesc.byteWidth = sizeof(BufferStruct::ConstantMVPBuffer);
    constantBufferDesc.debugName = "GridMVPConstantBuffer";
    m_ConstantBuffer = graphics.CreateBuffer(constantBufferDesc, nullptr);

    GraphicsShaderDesc vsDesc = {};
    vsDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/Grid_VS.hlsl");
    vsDesc.stage = GraphicsShaderStage::Vertex;
    vsDesc.entryPoint = "VS";
    vsDesc.debugName = "Grid_VS";

    GraphicsShaderDesc psDesc = {};
    psDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/Grid_PS.hlsl");
    psDesc.stage = GraphicsShaderStage::Pixel;
    psDesc.entryPoint = "PS";
    psDesc.debugName = "Grid_PS";

    auto vsBytecode = graphics.CompileShader(vsDesc);
    auto psBytecode = graphics.CompileShader(psDesc);
    if (!vsBytecode || !psBytecode)
    {
        MOON_LOG("Failed to compile grid shaders");
        return false;
    }

    m_VertexShader = graphics.CreateVertexShader(*vsBytecode, "Grid_VS");
    m_PixelShader = graphics.CreatePixelShader(*psBytecode, "Grid_PS");
    m_InputLayout = graphics.CreateInputLayout(BufferStruct::VertexPosColor::GetVertexLayout(), *vsBytecode, "GridInputLayout");

    return m_VertexBuffer && m_IndexBuffer && m_ConstantBuffer && m_VertexShader && m_PixelShader && m_InputLayout;
}

void GridRenderer::Render(IGraphicsBackend& graphics, const BufferStruct::ConstantMVPBuffer& mvp)
{
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
