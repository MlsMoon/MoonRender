#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "D3DApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "Source/ThirdParty/ImGui/imgui_impl_win32.h"
#include "Source/ThirdParty/ImGui/imgui_impl_dx11.h"
#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"
#include "Source/UI/UserInterface.h"
#include "Source/EventSystem/EventCenter.h"
#include "Source/Core/public/MoonRenderClass.h"

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
    float GetCameraFOVValue();
    void SetCameraFOVValue(float newCameraFOV);
    
//private function:
private:
    bool InitResources();
    bool InitShaders();
    float CameraFOVValue = 90.0f;

//private parameter
private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;	
    ComPtr<ID3D11Buffer> m_pVertexBuffer;		
    ComPtr<ID3D11Buffer> m_pIndexBuffer;
    ComPtr<ID3D11Buffer> m_pConstantBuffer;
    
    ComPtr<ID3D11VertexShader> m_pVertexShader;	
    ComPtr<ID3D11PixelShader> m_pPixelShader;
    BufferStruct::ConstantMVPBuffer m_CBuffer;
};


#endif