#include "../public/GameApp.h"


// Game的部分
GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),m_CBuffer()
{
    if(GameApp::flag_exist == true)
        return;
    GameApp::flag_exist = true;
    GameApp::currentGameApp = this;
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
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
    static float phi = 0.0f, theta = 0.0f;
    phi += 0.3f * dt, theta += 0.37f * dt;
    m_CBuffer.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta));
    m_CBuffer.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(CameraFOVValue), AspectRatio(), 1.0f, 1000.0f));
    // m_CBuffer.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationX(phi));
    // 更新常量缓冲区，让立方体转起来
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    //memcpy_s() 是 C/C++ 标准库中的一个函数,用于执行内存拷贝操作。它用于安全地将一个内存区域的数据复制到另一个内存区域。
    memcpy_s(mappedData.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float blue[4] = { 0.1f, 0.1f, 0.1f, 1.0f };  // RGBA = (0,0,255,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制
    m_pd3dImmediateContext->DrawIndexed(36, 0, 0);

    //绘制UI
    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
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

    //载入顶点数据
    //这里先用一个三角形代替，后续需要载入模型获取顶点数据

    // 设置三角形顶点
    const BufferStruct::VertexPosColor vertices[] =
    {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }
    };

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
    vertex_buffer_desc.ByteWidth = sizeof vertices;
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
    InitData.pSysMem = vertices;
    HR(m_pd3dDevice->CreateBuffer(&vertex_buffer_desc, &InitData, m_pVertexBuffer.GetAddressOf()));



    // ******************
    // 索引数组
    //
    DWORD indices[] = {
        // 正面
        0, 1, 2,
        2, 3, 0,
        // 左面
        4, 5, 1,
        1, 0, 4,
        // 顶面
        1, 5, 6,
        6, 2, 1,
        // 背面
        7, 6, 5,
        5, 4, 7,
        // 右面
        3, 2, 6,
        6, 7, 3,
        // 底面
        4, 0, 3,
        3, 7, 4
    };
    // 设置索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof indices;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = indices;
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
    // 新建常量缓冲区，不使用初始数据
    HR(m_pd3dDevice->CreateBuffer(&constant_buffer_desc, nullptr, m_pConstantBuffer.GetAddressOf()));

    // 初始化常量缓冲区的值
    //MVP
    m_CBuffer.world = DirectX::XMMatrixIdentity();	// 单位矩阵的转置是它本身
    m_CBuffer.view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_CBuffer.proj = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(CameraFOVValue), AspectRatio(), 1.0f, 1000.0f));

    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(BufferStruct::VertexPosColor);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // 设置图元类型
    // 图元类似：定义了顶点数据如何描述一个几何形状
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //设置输入布局
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());



    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Trangle_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Trangle_PS");
    

    return true;
}

bool GameApp::InitShaders()
{
    ComPtr<ID3DBlob> blob_vertex;
    ComPtr<ID3DBlob> blob_pixel;
    
    //编译Shader
    HR(MoonCreateShaderFromFile(L"HLSL\\Example\\Cube\\Cube_VS.hlsl",CompileShaderType::VS, blob_vertex.GetAddressOf()));
    HR(MoonCreateShaderFromFile(L"HLSL\\Example\\Cube\\Cube_PS.hlsl",CompileShaderType::PS, blob_pixel.GetAddressOf()));
    
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
    HR(m_pd3dDevice->CreateInputLayout(BufferStruct::VertexPosColor::inputLayout, ARRAYSIZE(BufferStruct::VertexPosColor::inputLayout),
        blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    return true;
}


void GameApp::SetCameraFOVValue(float newCameraFOV)
{
    CameraFOVValue = newCameraFOV;
}
