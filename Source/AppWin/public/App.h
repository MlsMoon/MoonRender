#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include "D3DApp.h"
#include "Source/AppWin/public/D3DUtil.h"
#include "Source/AppWin/public/DXTrace.h"
#include "Source/Logging/public/LogSystem.h"
#include "Source/Object/MoonObject.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"
#include "Source/UI/UserInterface.h"

class App : public D3DApp
{
public:
    App(
        HINSTANCE hInstance,
        const std::wstring& windowName,
        int initWidth,
        int initHeight,
        GraphicsBackendType backendType = GraphicsBackendType::DX11);
    ~App() override;

    inline static App* currentApp = nullptr;
    inline static bool flag_exist = false;

    MoonUI::UserInterface user_interface;
    Logging::LogSystem log_system;

    bool Init() override;
    void OnResize() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;
    void DrawUI() override;

    const std::vector<std::unique_ptr<Object::MoonObject>>& GetSceneObjects() const { return m_sceneObjects; }
    Object::MoonObject*& GetSelectedObject() { return m_selectedObject; }

private:
    void CreateDefaultScene();
    bool InitResources();
    bool InitShaders();
    Object::MoonObject* FindFirstRenderableObject();
    Object::MoonObject* FindMainCameraObject();
    Object::MoonObject* FindDirectionalLightObject();

private:
    std::vector<std::unique_ptr<Object::MoonObject>> m_sceneObjects;
    Object::MoonObject* m_selectedObject = nullptr;
    Object::MoonObject* m_renderObject = nullptr;
    Object::MoonObject* m_mainCameraObject = nullptr;
    Object::MoonObject* m_directionalLightObject = nullptr;

    std::shared_ptr<IGraphicsInputLayout> m_VertexLayout;
    std::shared_ptr<IGraphicsBuffer> m_VertexBuffer;
    std::shared_ptr<IGraphicsBuffer> m_IndexBuffer;
    std::shared_ptr<IGraphicsBuffer> m_ConstantBuffers[2];
    std::uint32_t m_IndexCount = 0;

    BufferStruct::ConstantMVPBuffer m_cBuffer_MVP;
    BufferStruct::ConstantPSBuffer m_cBuffer_PS;
    std::shared_ptr<IGraphicsVertexShader> m_VertexShader;
    std::shared_ptr<IGraphicsPixelShader> m_PixelShader;

    std::shared_ptr<IGraphicsRasterizerState> m_WireframeRasterizerState;
    bool m_IsWireframeMode;
};

#endif
