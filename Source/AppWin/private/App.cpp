#include "../public/App.h"

#include <vector>

#include "Source/Logging/public/LogSystem.h"
#include "Source/Object/AutoRotateComponent.h"
#include "Source/Object/CameraComponent.h"
#include "Source/Object/LightComponent.h"
#include "Source/Object/MeshComponent.h"
#include "Source/Object/TransformComponent.h"
#include "Source/ThirdParty/ImGui/imgui.h"

App::App(
    HINSTANCE hInstance,
    const std::wstring& windowName,
    int initWidth,
    int initHeight,
    GraphicsBackendType backendType)
    : D3DApp(hInstance, windowName, initWidth, initHeight, backendType)
{
    if (App::flag_exist)
    {
        return;
    }

    App::flag_exist = true;
    App::currentApp = this;

    m_cBuffer_MVP = BufferStruct::ConstantMVPBuffer();
    m_cBuffer_PS = BufferStruct::ConstantPSBuffer();
    m_IsWireframeMode = false;
}

App::~App() = default;

bool App::Init()
{
    MOON_LOG("Hello");
    MOON_LOG("Start Init");
    user_interface.BindLogSystem(&log_system);
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

void App::OnResize()
{
    D3DApp::OnResize();
}

void App::UpdateScene(float dt)
{
    for (const auto& object : m_sceneObjects)
    {
        for (const auto& component : object->GetComponents())
        {
            component->Update(*object, dt);
        }
    }

    Object::TransformComponent* renderTransform = nullptr;
    if (m_renderObject != nullptr)
    {
        renderTransform = m_renderObject->GetComponent<Object::TransformComponent>();
    }

    const DirectX::XMMATRIX world = renderTransform != nullptr
        ? renderTransform->GetWorldMatrix()
        : DirectX::XMMatrixIdentity();

    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    float cameraFovRadians = DirectX::XMConvertToRadians(90.0f);
    float cameraNearPlane = 1.0f;
    float cameraFarPlane = 1000.0f;
    if (m_mainCameraObject != nullptr)
    {
        const auto* cameraTransform = m_mainCameraObject->GetComponent<Object::TransformComponent>();
        auto* cameraComponent = m_mainCameraObject->GetComponent<Object::CameraComponent>();
        if (cameraTransform != nullptr && cameraComponent != nullptr)
        {
            cameraComponent->Normalize();
            const DirectX::XMMATRIX cameraRotation =
                DirectX::XMMatrixRotationX(cameraTransform->rotationRadians.x) *
                DirectX::XMMatrixRotationY(cameraTransform->rotationRadians.y) *
                DirectX::XMMatrixRotationZ(cameraTransform->rotationRadians.z);
            const DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&cameraTransform->position);
            const DirectX::XMVECTOR forward = DirectX::XMVector3TransformNormal(
                DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
                cameraRotation);
            const DirectX::XMVECTOR up = DirectX::XMVector3TransformNormal(
                DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
                cameraRotation);
            view = DirectX::XMMatrixLookAtLH(cameraPosition, DirectX::XMVectorAdd(cameraPosition, forward), up);
            cameraFovRadians = cameraComponent->fovRadians;
            cameraNearPlane = cameraComponent->nearPlane;
            cameraFarPlane = cameraComponent->farPlane;
        }
    }

    if (m_directionalLightObject != nullptr)
    {
        const auto* lightComponent = m_directionalLightObject->GetComponent<Object::LightComponent>();
        if (lightComponent != nullptr && lightComponent->GetLightKind() == Object::LightKind::Directional)
        {
            m_cBuffer_PS.directionalLightDirW = lightComponent->directionalLight.direction_intensity;
        }
    }

    m_cBuffer_MVP.world = DirectX::XMMatrixTranspose(world);
    m_cBuffer_MVP.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(world));
    m_cBuffer_MVP.view = DirectX::XMMatrixTranspose(view);
    m_cBuffer_MVP.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(
        cameraFovRadians,
        AspectRatio(),
        cameraNearPlane,
        cameraFarPlane));

    IGraphicsBackend& graphics = Graphics();
    if (m_ConstantBuffers[0] && m_ConstantBuffers[1])
    {
        graphics.UpdateBuffer(*m_ConstantBuffers[0], &m_cBuffer_MVP, sizeof(m_cBuffer_MVP));
        graphics.UpdateBuffer(*m_ConstantBuffers[1], &m_cBuffer_PS, sizeof(m_cBuffer_PS));
    }
}

void App::DrawScene()
{
    static const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    IGraphicsBackend& graphics = Graphics();
    graphics.Clear(clearColor, 1.0f, 0);
    if (m_IndexCount > 0)
    {
        graphics.DrawIndexed(m_IndexCount, 0, 0);
    }

    ImGui::Render();
    graphics.RenderImGuiDrawData();
    graphics.Present();
}

void App::DrawUI()
{
    user_interface.DrawMainInterfaceUI(m_sceneObjects, m_selectedObject, GetGraphicsBackendType());
}

void App::CreateDefaultScene()
{
    m_sceneObjects.clear();
    m_selectedObject = nullptr;
    m_renderObject = nullptr;
    m_mainCameraObject = nullptr;
    m_directionalLightObject = nullptr;

    auto sphereObject = std::make_unique<Object::MoonObject>("Sphere");
    sphereObject->AddComponent<Object::TransformComponent>();
    sphereObject->AddComponent<Object::MeshComponent>(MoonGetAssetPath("Resources/Models/sphere.obj"), ResourcesProcess::OBJ);
    sphereObject->AddComponent<Object::AutoRotateComponent>();
    m_selectedObject = sphereObject.get();
    m_sceneObjects.push_back(std::move(sphereObject));

    auto cameraObject = std::make_unique<Object::MoonObject>("Main Camera");
    Object::TransformComponent* cameraTransform = cameraObject->AddComponent<Object::TransformComponent>();
    if (cameraTransform != nullptr)
    {
        cameraTransform->position = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
    }
    Object::CameraComponent* cameraComponent = cameraObject->AddComponent<Object::CameraComponent>();
    if (cameraComponent != nullptr)
    {
        cameraComponent->fovRadians = DirectX::XMConvertToRadians(90.0f);
        cameraComponent->nearPlane = 1.0f;
        cameraComponent->farPlane = 1000.0f;
        cameraComponent->Normalize();
    }
    m_sceneObjects.push_back(std::move(cameraObject));

    auto lightObject = std::make_unique<Object::MoonObject>("Directional Light");
    lightObject->AddComponent<Object::TransformComponent>();
    lightObject->AddComponent<Object::LightComponent>(Object::LightKind::Directional);
    m_sceneObjects.push_back(std::move(lightObject));

    m_renderObject = FindFirstRenderableObject();
    m_mainCameraObject = FindMainCameraObject();
    m_directionalLightObject = FindDirectionalLightObject();
}

bool App::InitResources()
{
    if (!InitShaders())
    {
        return false;
    }

    CreateDefaultScene();
    if (m_renderObject == nullptr)
    {
        return false;
    }

    Object::MeshComponent* meshComponent = m_renderObject->GetComponent<Object::MeshComponent>();
    if (meshComponent == nullptr || meshComponent->GetMesh() == nullptr || meshComponent->GetMesh()->VertexNum == 0)
    {
        return false;
    }

    ResourcesProcess::Mesh* defaultMesh = meshComponent->GetMesh();

    IGraphicsBackend& graphics = Graphics();

    GraphicsBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.byteWidth = defaultMesh->ByteWidth;
    vertexBufferDesc.usage = GraphicsBufferUsage::Immutable;
    vertexBufferDesc.bindFlags = GraphicsBufferBindFlags::VertexBuffer;
    vertexBufferDesc.debugName = "VertexBuffer";
    m_VertexBuffer = graphics.CreateBuffer(vertexBufferDesc, defaultMesh->get_sys_mem());

    std::vector<std::uint32_t> indices(defaultMesh->VertexNum);
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(defaultMesh->VertexNum); ++i)
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
    m_cBuffer_MVP.view = DirectX::XMMatrixIdentity();
    m_cBuffer_MVP.proj = DirectX::XMMatrixIdentity();

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

bool App::InitShaders()
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

float App::GetCameraFOVValue()
{
    Object::MoonObject* cameraObject = FindMainCameraObject();
    if (cameraObject == nullptr)
    {
        return 90.0f;
    }

    auto* cameraComponent = cameraObject->GetComponent<Object::CameraComponent>();
    if (cameraComponent == nullptr)
    {
        return 90.0f;
    }

    cameraComponent->Normalize();
    return DirectX::XMConvertToDegrees(cameraComponent->fovRadians);
}

void App::SetCameraFOVValue(float newCameraFOV)
{
    Object::MoonObject* cameraObject = FindMainCameraObject();
    if (cameraObject == nullptr)
    {
        return;
    }

    auto* cameraComponent = cameraObject->GetComponent<Object::CameraComponent>();
    if (cameraComponent != nullptr)
    {
        cameraComponent->fovRadians = DirectX::XMConvertToRadians(newCameraFOV);
        cameraComponent->Normalize();
    }
}

Object::MoonObject* App::FindFirstRenderableObject()
{
    for (const auto& object : m_sceneObjects)
    {
        if (object->HasComponent<Object::TransformComponent>() && object->HasComponent<Object::MeshComponent>())
        {
            return object.get();
        }
    }
    return nullptr;
}

Object::MoonObject* App::FindMainCameraObject()
{
    for (const auto& object : m_sceneObjects)
    {
        if (object->HasComponent<Object::TransformComponent>() && object->HasComponent<Object::CameraComponent>())
        {
            return object.get();
        }
    }
    return nullptr;
}

Object::MoonObject* App::FindDirectionalLightObject()
{
    for (const auto& object : m_sceneObjects)
    {
        const auto* lightComponent = object->GetComponent<Object::LightComponent>();
        if (object->HasComponent<Object::TransformComponent>() &&
            lightComponent != nullptr &&
            lightComponent->GetLightKind() == Object::LightKind::Directional)
        {
            return object.get();
        }
    }
    return nullptr;
}
