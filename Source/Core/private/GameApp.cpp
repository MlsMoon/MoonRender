#include "../public/GameApp.h"


// Game的部分
GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight)
{
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

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{

}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);
    static float blue[4] = { 0.1f, 0.1f, 0.1f, 1.0f };  // RGBA = (0,0,255,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制三角形
    m_pd3dImmediateContext->Draw(8, 0);

    //绘制UI
    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawUI()
{
    //ImGui::SetNextWindowSize(ImVec2(400, 600));
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open"))
        {
            // 处理 "Open" 被点击的逻辑
        }

        if (ImGui::MenuItem("Save"))
        {
            // 处理 "Save" 被点击的逻辑
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows"))
    {
        if (ImGui::MenuItem("Test"))
        {
            // 处理 "Open" 被点击的逻辑
        }

        //if (ImGui::MenuItem("Save"))
        //{
        //    // 处理 "Save" 被点击的逻辑
        //}
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

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
    
    // 设置Buffer描述
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
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(BufferStruct::VertexPosColor);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    
    // 设置图元类型
    // 图元类似：定义了顶点数据如何描述一个几何形状
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //设置输入布局
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

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
    HR(MoonCreateShaderFromFile(L"HLSL\\Example\\Triangle\\Triangle_VS.hlsl",CompileShaderType::VS,"VS","vs_5_0", blob_vertex.GetAddressOf()));
    HR(MoonCreateShaderFromFile(L"HLSL\\Example\\Triangle\\Triangle_PS.hlsl",CompileShaderType::PS,"PS","ps_5_0", blob_pixel.GetAddressOf()));
    
    //创建 顶点着色器
    HR(m_pd3dDevice->CreateVertexShader(blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
    //创建像素着色器
    HR(m_pd3dDevice->CreatePixelShader(blob_pixel->GetBufferPointer(), blob_pixel->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

    //
    
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
