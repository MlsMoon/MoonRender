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
#include "Source/AppWin//public/MoonRenderClass.h"
#include "Source/EventSystem/LogSystem.h"
#include "Source/ResourcesProcess/public/MoonMeshLoader.h"
#include "Source/ThirdParty/tinyobjloader/tiny_obj_loader.h"

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    inline static GameApp* currentGameApp = nullptr;
    bool flag_exist = false;
    
    MoonUI::UserInterface game_user_interface;
    EventSystem::LogSystem log_system;

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
    void DrawUI();
    float GetCameraFOVValue();
    void SetCameraFOVValue(float newCameraFOV);
    float CameraFOVValue = 90.0f;
    
//private function:
private:
    bool InitResources();
    bool InitShaders();


//private parameter
private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout;	
    ComPtr<ID3D11Buffer> m_pVertexBuffer;		
    ComPtr<ID3D11Buffer> m_pIndexBuffer;
    ComPtr<ID3D11Buffer> m_pConstantBuffers[2];
    UINT m_IndexCount;
    
    BufferStruct::ConstantMVPBuffer m_cBuffer_MVP;			// 用于修改用于VS的GPU常量缓冲区的变量
    BufferStruct::ConstantPSBuffer m_cBuffer_PS;			// 用于修改用于PS的GPU常量缓冲区的变量
    ComPtr<ID3D11VertexShader> m_pVertexShader;	
    ComPtr<ID3D11PixelShader> m_pPixelShader;

    Render::DirectionalLight m_DirLight;					// 默认环境光
    Render::PointLight m_PointLight;						// 默认点光
    Render::SpotLight m_SpotLight;						    // 默认汇聚光
    
    ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
    bool m_IsWireframeMode;							// 当前是否为线框模式
};


#endif