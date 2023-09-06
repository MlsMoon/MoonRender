#include "../public/GameApp.h"

#include "Source/Logging/public/LogSystem.h"

// Game的部分
GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
    if(GameApp::flag_exist == true)
        return;
    GameApp::flag_exist = true;
    GameApp::currentGameApp = this;
    
    m_cBuffer_MVP = BufferStruct::ConstantMVPBuffer();
    m_cBuffer_PS = BufferStruct::ConstantPSBuffer();
    m_DirLight = Render::DirectionalLight();
    m_PointLight = Render::PointLight();
    m_SpotLight = Render::SpotLight();
    m_IsWireframeMode = false;

    std::string currentFilePath = __FILE__;
    project_root_path = PROJECT_ROOT_PATH(currentFilePath);
}

GameApp::~GameApp()
{
    delete default_mesh;
}

bool GameApp::Init()
{
    MOON_LOG("Hello");
    MOON_LOG("Start Init");
    if (!D3DApp::Init())
        return false;

    if (!GameApp::InitResources())
        return false;
    
    const MoonFunctionPtr<float> function_ptr_set_camera_fov = [this](float value) { SetCameraFOVValue(value); };
    EventCenter::AddListener("SetCameraFOVValue",function_ptr_set_camera_fov);

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
    
    //一个转动MVP矩阵的效果
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt, theta += 0.37f * dt;
    
    DirectX::XMMATRIX W = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);
    m_cBuffer_MVP.world = XMMatrixTranspose(W);
    m_cBuffer_MVP.worldInvTranspose = DirectX::XMMatrixTranspose(InverseTranspose(W));
    m_cBuffer_MVP.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(CameraFOVValue), AspectRatio(), 1.0f, 1000.0f));
    // m_CBuffer.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationX(phi));
    // 更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(BufferStruct::ConstantMVPBuffer), &m_cBuffer_MVP, sizeof(m_cBuffer_MVP));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);

    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(BufferStruct::ConstantPSBuffer), &m_cBuffer_PS, sizeof(m_cBuffer_PS));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);

    
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float blue[4] = { 0.1f, 0.1f, 0.1f, 1.0f };  
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制
    m_pd3dImmediateContext->DrawIndexed(static_cast<UINT>(default_mesh->VertexNum), 0, 0);

    //绘制UI
    ImGui::Render();
    // 触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawUI()
{
    game_user_interface.DrawMainInterfaceUI();
}

bool GameApp::InitResources()
{
    if (!InitShaders())
        return false;

    //通过obj文件，载入顶点数据
    
    const std::string mesh_file_path = project_root_path + "Resources\\Models\\Sphere.obj";
    default_mesh = new ResourcesProcess::Mesh(mesh_file_path,ResourcesProcess::OBJ);
    
    // 顶点缓冲区描述 结构如下：
    // typedef struct D3D11_BUFFER_DESC
    // {
    //     UINT ByteWidth;             // 数据字节数
    //     D3D11_USAGE Usage;          // CPU和GPU的读写权限相关
    //     UINT BindFlags;             // 缓冲区类型的标志
    //     UINT CPUAccessFlags;        // CPU读写权限的指定
    //     UINT MiscFlags;             // 忽略
    //     UINT StructureByteStride;   // 忽略
    // }     D3D11_BUFFER_DESC;
    
    // 设置Vertex Buffer描述
    D3D11_BUFFER_DESC vertex_buffer_desc;
    // 是一个常见的用法，用于将内存块的内容设置为零。
    //在这个语句中，ZeroMemory 是一个宏定义，它接受两个参数：要清零的内存块的起始地址和内存块的大小。
    ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
    vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; // 设置 cpu和gpu 的读写权限，不同权限的更新效率不一样
    vertex_buffer_desc.ByteWidth = default_mesh->ByteWidth;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 设置buffer 类型 这里设置为 vertex buffer
    vertex_buffer_desc.CPUAccessFlags = 0;
    
    // ，许多资源（如纹理、顶点缓冲区等）需要在创建时进行初始化，以提供初始的数据。
    // typedef struct D3D11_SUBRESOURCE_DATA {
    //     const void* pSysMem;        // 指向初始化数据的指针
    //     UINT SysMemPitch;           // 数据的行大小（以字节为单位），对于二维纹理数据有效
    //     UINT SysMemSlicePitch;      // 数据的深度大小（以字节为单位），对于三维纹理数据有效
    // } D3D11_SUBRESOURCE_DATA;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = default_mesh->get_sys_mem();
    HR(m_pd3dDevice->CreateBuffer(&vertex_buffer_desc, &InitData, m_pVertexBuffer.GetAddressOf()));



    // ******************
    // 索引数组
    //
    std::vector<DWORD> indices_mesh;
    indices_mesh.resize(default_mesh->VertexNum);
    for (int i = 0 ; i < default_mesh->VertexNum;i++)
    {
        indices_mesh[i] = i;
    }
    // 设置索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = indices_mesh.size() * 4;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = indices_mesh.data();
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));

    // ******************
    // 设置常量缓冲区描述
    //
    D3D11_BUFFER_DESC constant_buffer_desc;
    ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.ByteWidth = sizeof(BufferStruct::ConstantMVPBuffer);
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // 新建常量缓冲区
    HR(m_pd3dDevice->CreateBuffer(&constant_buffer_desc, nullptr, m_pConstantBuffers[0].GetAddressOf()));
    constant_buffer_desc.ByteWidth = sizeof(BufferStruct::ConstantPSBuffer);
    HR(m_pd3dDevice->CreateBuffer(&constant_buffer_desc, nullptr, m_pConstantBuffers[1].GetAddressOf()));

    // 初始化常量缓冲区的值
    //MVP
    //TODO:可移动
    m_cBuffer_MVP.world = DirectX::XMMatrixIdentity();	// 单位矩阵的转置是它本身
    m_cBuffer_MVP.view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_cBuffer_MVP.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(CameraFOVValue), AspectRatio(), 1.0f, 1000.0f));

    // ******************
    // 初始化默认光照

    // 初始化用于PS的常量缓冲区的值
    m_cBuffer_PS.directionalLightDirW =  DirectX::XMFLOAT4(-0.577f, -0.577f, 0.577f,1.0f);
    

    // ******************
    // 初始化光栅化状态
    //
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthClipEnable = true;
    HR(m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));
    
    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(BufferStruct::VertexPosNormal);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // 设置图元类型
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //设置输入布局
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());



    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "MVPConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Light_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Light_PS");
    

    return true;
}

bool GameApp::InitShaders()
{
    ComPtr<ID3DBlob> blob_vertex;
    ComPtr<ID3DBlob> blob_pixel;
    
    //编译Shader
    HR(MoonCreateShaderFromFile(L"Resources\\Shaders\\VertexCommon.hlsl",CompileShaderType::VS, blob_vertex.GetAddressOf()));
    HR(MoonCreateShaderFromFile(L"Resources\\Shaders\\Light_PS.hlsl",CompileShaderType::PS, blob_pixel.GetAddressOf()));
    // HR(MoonCreateShaderFromFile(L"Resources\\Shaders\\Example\\Cube\\Cube_VS.hlsl",CompileShaderType::VS, blob_vertex.GetAddressOf()));
    // HR(MoonCreateShaderFromFile(L"Resources\\Shaders\\Example\\Cube\\Cube_PS.hlsl",CompileShaderType::PS, blob_pixel.GetAddressOf()));
    
    //创建 顶点着色器
    HR(m_pd3dDevice->CreateVertexShader(blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
    //创建像素着色器
    HR(m_pd3dDevice->CreatePixelShader(blob_pixel->GetBufferPointer(), blob_pixel->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));
    
    // HRESULT ID3D11Device::CreateInputLayout( 
    // const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, // [In]输入布局描述
    // UINT NumElements,                                   // [In]上述数组元素个数
    // const void *pShaderBytecodeWithInputSignature,      // [In]顶点着色器字节码
    // SIZE_T BytecodeLength,                              // [In]顶点着色器字节码长度
    // ID3D11InputLayout **ppInputLayout);                 // [Out]获取的输入布局
    
    // 创建并绑定顶点布局
    // 顶点布局只需要传给顶点着色器
    HR(m_pd3dDevice->CreateInputLayout(BufferStruct::VertexPosNormal::inputLayout, ARRAYSIZE(BufferStruct::VertexPosNormal::inputLayout),
        blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    return true;
}


void GameApp::SetCameraFOVValue(float newCameraFOV)
{
    CameraFOVValue = newCameraFOV;
}
