#ifndef APP_H
#define APP_H

#include <memory>
#include <vector>

#include "D3DApp.h"
#include "Source/AppWin/public/D3DUtil.h"
#include "Source/AppWin/public/DXTrace.h"
#include "Source/Logging/public/LogSystem.h"
#include "Source/Object/Scene.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"
#include "Source/UI/Editor.h"

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

    MoonUI::Editor editor;
    Logging::LogSystem log_system;

    bool Init() override;
    void OnResize() override;
    void UpdateScene(float dt) override;
    void DrawScene() override;
    void DrawUI() override;

    Object::Scene* GetScene() const { return m_scene.get(); }
    Object::MoonObject*& GetSelectedObject() { return m_selectedObject; }

private:
    bool InitResources();
    bool InitShaders();

private:
    std::unique_ptr<Object::Scene> m_scene;
    Object::MoonObject* m_selectedObject = nullptr;

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
