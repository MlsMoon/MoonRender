#ifndef D3DAPP_H
#define D3DAPP_H

#include <memory>
#include <string>

#include "CpuTimer.h"
#include "WinMin.h"
#include "Source/Graphics/public/GraphicsBackend.h"

class D3DApp
{
public:
    D3DApp(
        HINSTANCE hInstance,
        const std::wstring& windowName,
        int initWidth,
        int initHeight,
        GraphicsBackendType backendType = GraphicsBackendType::DX11);
    virtual ~D3DApp();

    HINSTANCE AppInst() const;
    HWND MainWnd() const;
    float AspectRatio() const;
    GraphicsBackendType GetGraphicsBackendType() const;

    int Run();

    virtual bool Init();
    virtual void OnResize();
    virtual void DrawUI();
    virtual void UpdateScene(float dt) = 0;
    virtual void DrawScene() = 0;
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    int ClientWidth;
    int ClientHeight;

protected:
    bool InitMainWindow();
    bool InitGraphicsBackend();
    bool InitImGui();
    void CalculateFrameStats();

    IGraphicsBackend& Graphics();
    const IGraphicsBackend& Graphics() const;

protected:
    HINSTANCE m_hAppInst;
    HWND m_hMainWnd;
    bool m_AppPaused;
    bool m_Minimized;
    bool m_Maximized;
    bool m_Resizing;
    bool m_Enable4xMsaa;

    CpuTimer m_Timer;
    std::wstring m_MainWndCaption;
    GraphicsBackendType m_GraphicsBackendType;
    std::unique_ptr<IGraphicsBackend> m_GraphicsBackend;
};

#endif
