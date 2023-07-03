#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "D3DApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "../ImGui/imgui.h"
#include "../ResourcesProcess/public/BufferStruct.h"

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
    void DrawUI();

//private function:
private:
    bool InitResources();
    bool InitShaders();

//private parameter
private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;	// �������벼��
    ComPtr<ID3D11Buffer> m_pVertexBuffer;		// ���㻺����
    ComPtr<ID3D11VertexShader> m_pVertexShader;	// ������ɫ��
    ComPtr<ID3D11PixelShader> m_pPixelShader;	// ������ɫ��
};


#endif