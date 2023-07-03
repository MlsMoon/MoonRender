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
    static float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };  // RGBA = (0,0,255,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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


    return true;
}

bool GameApp::InitShaders()
{
    ComPtr<ID3DBlob> blob_vertex;
    ComPtr<ID3DBlob> blob_pixel;
    
    //载入编译好的Shader
    HR(D3DReadFileToBlob(L"HLSL\\CSO\\Triangle_VS.cso", blob_vertex.GetAddressOf()));
    HR(D3DReadFileToBlob(L"HLSL\\CSO\\Triangle_PS.cso", blob_pixel.GetAddressOf()));
    
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
