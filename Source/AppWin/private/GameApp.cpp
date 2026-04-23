#include "../public/GameApp.h"

#include <vector>

#include "Source/Logging/public/LogSystem.h"
#include "Source/ThirdParty/ImGui/imgui.h"

GameApp::GameApp(
    HINSTANCE hInstance,
    const std::wstring& windowName,
    int initWidth,
    int initHeight,
    GraphicsBackendType backendType)
    : D3DApp(hInstance, windowName, initWidth, initHeight, backendType)
{
    if (GameApp::flag_exist)
    {
        return;
    }

    GameApp::flag_exist = true;
    GameApp::currentGameApp = this;

    m_cBuffer_MVP = BufferStruct::ConstantMVPBuffer();
    m_cBuffer_PS = BufferStruct::ConstantPSBuffer();
    m_DirLight = Render::DirectionalLight();
    m_PointLight = Render::PointLight();
    m_SpotLight = Render::SpotLight();
    m_IsWireframeMode = false;
}

GameApp::~GameApp() = default;

bool GameApp::Init()
{
    MOON_LOG("Hello");
    MOON_LOG("Start Init");
    if (!D3DApp::Init())
    {
        return false;
    }

    if (!InitResources())
    {
        return false;
    }

    const MoonFunctionPtr<float> function_ptr_set_camera_fov = [this](float value) { SetCameraFOVValue(value); };
    EventCenter::AddListener("SetCameraFOVValue", function_ptr_set_camera_fov);
    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    static float phi = 0.0f;
    static float theta = 0.0f;
    phi += 0.3f * dt;
    theta += 0.37f * dt;

    DirectX::XMMATRIX world = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);
    m_cBuffer_MVP.world = DirectX::XMMatrixTranspose(world);
    m_cBuffer_MVP.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(world));
    m_cBuffer_MVP.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(CameraFOVValue),
        AspectRatio(),
        1.0f,
        1000.0f));

    IGraphicsBackend& graphics = Graphics();
    graphics.UpdateBuffer(*m_ConstantBuffers[0], &m_cBuffer_MVP, sizeof(m_cBuffer_MVP));
    graphics.UpdateBuffer(*m_ConstantBuffers[1], &m_cBuffer_PS, sizeof(m_cBuffer_PS));
}

void GameApp::DrawScene()
{
    static const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    IGraphicsBackend& graphics = Graphics();
    graphics.Clear(clearColor, 1.0f, 0);
    graphics.DrawIndexed(m_IndexCount, 0, 0);

    ImGui::Render();
    graphics.RenderImGuiDrawData();
    graphics.Present();
}

void GameApp::DrawUI()
{
    game_user_interface.DrawMainInterfaceUI();
}

bool GameApp::InitResources()
{
    if (!InitShaders())
    {
        return false;
    }

    const std::string meshFilePath = MoonGetAssetPath("Resources/Models/sphere.obj");
    default_mesh = std::make_unique<ResourcesProcess::Mesh>(meshFilePath, ResourcesProcess::OBJ);

    IGraphicsBackend& graphics = Graphics();

    GraphicsBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.byteWidth = default_mesh->ByteWidth;
    vertexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    vertexBufferDesc.bindFlags = GraphicsBufferBindFlags::VertexBuffer;
    vertexBufferDesc.debugName = "VertexBuffer";
    m_VertexBuffer = graphics.CreateBuffer(vertexBufferDesc, default_mesh->get_sys_mem());

    std::vector<std::uint32_t> indices(default_mesh->VertexNum);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(default_mesh->VertexNum); ++i)
    {
        indices[i] = i;
    }
    m_IndexCount = static_cast<std::uint32_t>(indices.size());

    GraphicsBufferDesc indexBufferDesc = {};
    indexBufferDesc.byteWidth = static_cast<std::uint32_t>(indices.size() * sizeof(std::uint32_t));
    indexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    indexBufferDesc.bindFlags = GraphicsBufferBindFlags::IndexBuffer;
    indexBufferDesc.debugName = "IndexBuffer";
    m_IndexBuffer = graphics.CreateBuffer(indexBufferDesc, indices.data());

    GraphicsBufferDesc constantBufferDesc = {};
    constantBufferDesc.usage = GraphicsBufferUsage::Dynamic;
    constantBufferDesc.bindFlags = GraphicsBufferBindFlags::ConstantBuffer;
    constantBufferDesc.cpuWrite = true;

    constantBufferDesc.byteWidth = sizeof(BufferStruct::ConstantMVPBuffer);
    constantBufferDesc.debugName = "MVPConstantBuffer";
    m_ConstantBuffers[0] = graphics.CreateBuffer(constantBufferDesc, nullptr);

    constantBufferDesc.byteWidth = sizeof(BufferStruct::ConstantPSBuffer);
    constantBufferDesc.debugName = "PSConstantBuffer";
    m_ConstantBuffers[1] = graphics.CreateBuffer(constantBufferDesc, nullptr);

    m_cBuffer_MVP.world = DirectX::XMMatrixIdentity();
    m_cBuffer_MVP.view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    m_cBuffer_MVP.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(CameraFOVValue),
        AspectRatio(),
        1.0f,
        1000.0f));

    m_cBuffer_PS.directionalLightDirW = DirectX::XMFLOAT4(-0.577f, -0.577f, 0.577f, 1.0f);

    GraphicsRasterizerDesc rasterizerDesc = {};
    rasterizerDesc.fillMode = GraphicsFillMode::Wireframe;
    rasterizerDesc.cullMode = GraphicsCullMode::None;
    rasterizerDesc.depthClipEnable = true;
    rasterizerDesc.debugName = "WireframeRasterizer";
    m_WireframeRasterizerState = graphics.CreateRasterizerState(rasterizerDesc);

    graphics.SetVertexBuffer(*m_VertexBuffer, sizeof(BufferStruct::VertexPosNormal), 0);
    graphics.SetIndexBuffer(*m_IndexBuffer, GraphicsIndexFormat::UInt32, 0);
    graphics.SetPrimitiveTopology(GraphicsPrimitiveTopology::TriangleList);
    graphics.SetInputLayout(m_VertexLayout.get());
    graphics.SetVertexShader(m_VertexShader.get());
    graphics.SetPixelShader(m_PixelShader.get());
    graphics.SetVertexConstantBuffer(0, m_ConstantBuffers[0].get());
    graphics.SetPixelConstantBuffer(1, m_ConstantBuffers[1].get());

    return m_VertexBuffer && m_IndexBuffer && m_ConstantBuffers[0] && m_ConstantBuffers[1];
}

bool GameApp::InitShaders()
{
    IGraphicsBackend& graphics = Graphics();

    GraphicsShaderDesc vertexShaderDesc = {};
    vertexShaderDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/VertexCommon.hlsl");
    vertexShaderDesc.stage = GraphicsShaderStage::Vertex;
    vertexShaderDesc.debugName = "Light_VS";

    GraphicsShaderDesc pixelShaderDesc = {};
    pixelShaderDesc.filePath = MoonGetAssetPathW(L"Resources/Shaders/Light_PS.hlsl");
    pixelShaderDesc.stage = GraphicsShaderStage::Pixel;
    pixelShaderDesc.debugName = "Light_PS";

    std::shared_ptr<IGraphicsShaderBytecode> vertexBytecode = graphics.CompileShader(vertexShaderDesc);
    std::shared_ptr<IGraphicsShaderBytecode> pixelBytecode = graphics.CompileShader(pixelShaderDesc);
    if (!vertexBytecode || !pixelBytecode)
    {
        return false;
    }

    m_VertexShader = graphics.CreateVertexShader(*vertexBytecode, "Light_VS");
    m_PixelShader = graphics.CreatePixelShader(*pixelBytecode, "Light_PS");
    m_VertexLayout = graphics.CreateInputLayout(BufferStruct::VertexPosNormal::GetVertexLayout(), *vertexBytecode, "VertexPosNormalLayout");

    return m_VertexShader && m_PixelShader && m_VertexLayout;
}

float GameApp::GetCameraFOVValue()
{
    return CameraFOVValue;
}

void GameApp::SetCameraFOVValue(float newCameraFOV)
{
    CameraFOVValue = newCameraFOV;
}
