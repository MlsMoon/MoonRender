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
    static float blue[4] = { 0.1f, 0.1f, 0.1f, 1.0f };  // RGBA = (0,0,255,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), blue);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // ����������
    m_pd3dImmediateContext->Draw(3, 0);

    //����UI
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

    //���붥������
    //��������һ�������δ��棬������Ҫ����ģ�ͻ�ȡ��������

    // ���������ζ���
    const BufferStruct::VertexPosColor vertices[] =
    {
        { DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
    };

    // ���㻺�������� �ṹ���£�
    // typedef struct D3D11_BUFFER_DESC
    // {
    //     UINT ByteWidth;             // �����ֽ���
    //     D3D11_USAGE Usage;          // CPU��GPU�Ķ�дȨ�����
    //     UINT BindFlags;             // ���������͵ı�־
    //     UINT CPUAccessFlags;        // CPU��дȨ�޵�ָ��
    //     UINT MiscFlags;             // ����
    //     UINT StructureByteStride;   // ����
    // }     D3D11_BUFFER_DESC;
    
    // ����Buffer����
    D3D11_BUFFER_DESC vertex_buffer_desc;
    // ��һ���������÷������ڽ��ڴ�����������Ϊ�㡣
    //���������У�ZeroMemory ��һ���궨�壬����������������Ҫ������ڴ�����ʼ��ַ���ڴ��Ĵ�С��
    ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
    vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE; // ���� cpu��gpu �Ķ�дȨ�ޣ���ͬȨ�޵ĸ���Ч�ʲ�һ��
    vertex_buffer_desc.ByteWidth = sizeof vertices;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // ����buffer ���� ��������Ϊ vertex buffer
    vertex_buffer_desc.CPUAccessFlags = 0;

    // �������Դ�����������㻺�����ȣ���Ҫ�ڴ���ʱ���г�ʼ�������ṩ��ʼ�����ݡ�
    // typedef struct D3D11_SUBRESOURCE_DATA {
    //     const void* pSysMem;        // ָ���ʼ�����ݵ�ָ��
    //     UINT SysMemPitch;           // ���ݵ��д�С�����ֽ�Ϊ��λ�������ڶ�ά����������Ч
    //     UINT SysMemSlicePitch;      // ���ݵ���ȴ�С�����ֽ�Ϊ��λ����������ά����������Ч
    // } D3D11_SUBRESOURCE_DATA;
    // �½����㻺����
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    HR(m_pd3dDevice->CreateBuffer(&vertex_buffer_desc, &InitData, m_pVertexBuffer.GetAddressOf()));

    // ******************
    // ����Ⱦ���߸����׶ΰ󶨺�������Դ
    //

    // ����װ��׶εĶ��㻺��������
    UINT stride = sizeof(BufferStruct::VertexPosColor);	// ��Խ�ֽ���
    UINT offset = 0;						// ��ʼƫ����

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    
    // ����ͼԪ����
    // ͼԪ���ƣ������˶��������������һ��������״
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //�������벼��
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    
    // ����ɫ���󶨵���Ⱦ����
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // ******************
    // ���õ��Զ�����
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
