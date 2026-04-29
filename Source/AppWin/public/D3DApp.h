#ifndef D3DAPP_H
#define D3DAPP_H

#include <memory>
#include <string>

#include "CpuTimer.h"
#include "Source/Graphics/public/GraphicsBackend.h"

struct GLFWwindow;

class D3DApp
{
public:
    D3DApp(
        const std::string& windowName,
        int initWidth,
        int initHeight,
        GraphicsBackendType backendType = GraphicsBackendType::DX11);
    virtual ~D3DApp();

    GLFWwindow* GetWindow() const;
    float AspectRatio() const;
    GraphicsBackendType GetGraphicsBackendType() const;

    int Run();

    virtual bool Init();
    virtual void OnResize();
    virtual void DrawUI();

    void UpdateFramebufferSize();
    virtual void UpdateScene(float dt) = 0;
    virtual void RenderViewport() = 0;
    virtual void DrawScene() = 0;

    void RenderImGui();

    int ClientWidth;
    int ClientHeight;
    int FramebufferWidth;
    int FramebufferHeight;

    double MouseX = 0.0;
    double MouseY = 0.0;
    bool MouseButtonLeft = false;

protected:
    bool InitMainWindow();
    bool InitGraphicsBackend();
    bool InitImGui();
    void CalculateFrameStats();

    IGraphicsBackend& Graphics();
    const IGraphicsBackend& Graphics() const;

protected:
    GLFWwindow* m_window = nullptr;
    bool m_AppPaused = false;
    bool m_Minimized = false;
    bool m_Maximized = false;
    bool m_Resizing = false;
    bool m_Enable4xMsaa = true;

    CpuTimer m_Timer;
    std::string m_MainWndCaption;
    GraphicsBackendType m_GraphicsBackendType;
    std::unique_ptr<IGraphicsBackend> m_GraphicsBackend;
};

#endif
