#include "../public/GameApp.h"


// Game�Ĳ���
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
    // ������仰�ᴥ��ImGui��Direct3D�Ļ���
    // �����Ҫ�ڴ�֮ǰ���󱸻������󶨵���Ⱦ������
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
            // ���� "Open" ��������߼�
        }

        if (ImGui::MenuItem("Save"))
        {
            // ���� "Save" ��������߼�
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows"))
    {
        if (ImGui::MenuItem("Test"))
        {
            // ���� "Open" ��������߼�
        }

        //if (ImGui::MenuItem("Save"))
        //{
        //    // ���� "Save" ��������߼�
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
    
    //�������õ�Shader
    HR(D3DReadFileToBlob(L"HLSL\\CSO\\Triangle_VS.cso", blob_vertex.GetAddressOf()));
    HR(D3DReadFileToBlob(L"HLSL\\CSO\\Triangle_PS.cso", blob_pixel.GetAddressOf()));
    
    //���� ������ɫ��
    HR(m_pd3dDevice->CreateVertexShader(blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
    //����������ɫ��
    HR(m_pd3dDevice->CreatePixelShader(blob_pixel->GetBufferPointer(), blob_pixel->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

    //
    
    // HRESULT ID3D11Device::CreateInputLayout( 
    // const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, // [In]���벼������
    // UINT NumElements,                                   // [In]��������Ԫ�ظ���
    // const void *pShaderBytecodeWithInputSignature,      // [In]������ɫ���ֽ���
    // SIZE_T BytecodeLength,                              // [In]������ɫ���ֽ��볤��
    // ID3D11InputLayout **ppInputLayout);                 // [Out]��ȡ�����벼��
    
    // �������󶨶��㲼��
    // ���㲼��ֻ��Ҫ����������ɫ��
    HR(m_pd3dDevice->CreateInputLayout(BufferStruct::VertexPosColor::inputLayout, ARRAYSIZE(BufferStruct::VertexPosColor::inputLayout),
        blob_vertex->GetBufferPointer(), blob_vertex->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    return true;
}
