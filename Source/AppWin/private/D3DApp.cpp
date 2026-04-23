#include "../public/D3DApp.h"

#pragma warning(disable: 6031)

#include <cassert>
#include <sstream>

#include "../public/D3DUtil.h"
#include "Source/Graphics/public/GraphicsBackendFactory.h"
#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/ThirdParty/ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace
{
    D3DApp* g_pd3dApp = nullptr;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(
    HINSTANCE hInstance,
    const std::wstring& windowName,
    int initWidth,
    int initHeight,
    GraphicsBackendType backendType)
    : m_hAppInst(hInstance),
      m_hMainWnd(nullptr),
      m_AppPaused(false),
      m_Minimized(false),
      m_Maximized(false),
      m_Resizing(false),
      m_Enable4xMsaa(true),
      m_MainWndCaption(windowName),
      m_GraphicsBackendType(backendType),
      ClientWidth(initWidth),
      ClientHeight(initHeight)
{
    g_pd3dApp = this;
}

D3DApp::~D3DApp()
{
    if (ImGui::GetCurrentContext() != nullptr)
    {
        if (m_GraphicsBackend)
        {
            m_GraphicsBackend->ShutdownImGui();
        }
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

HINSTANCE D3DApp::AppInst() const
{
    return m_hAppInst;
}

HWND D3DApp::MainWnd() const
{
    return m_hMainWnd;
}

float D3DApp::AspectRatio() const
{
    return static_cast<float>(ClientWidth) / ClientHeight;
}

GraphicsBackendType D3DApp::GetGraphicsBackendType() const
{
    return m_GraphicsBackendType;
}

int D3DApp::Run()
{
    MSG msg = { 0 };
    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_Timer.Tick();

            if (!m_AppPaused)
            {
                CalculateFrameStats();

                Graphics().BeginImGuiFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                DrawUI();
                UpdateScene(m_Timer.DeltaTime());
                DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return static_cast<int>(msg.wParam);
}

bool D3DApp::Init()
{
    if (!InitMainWindow())
    {
        return false;
    }

    if (!InitGraphicsBackend())
    {
        return false;
    }

    if (!InitImGui())
    {
        return false;
    }

    return true;
}

void D3DApp::OnResize()
{
    if (m_GraphicsBackend)
    {
        m_GraphicsBackend->Resize(ClientWidth, ClientHeight);
    }
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            m_Timer.Stop();
        }
        else
        {
            m_AppPaused = false;
            m_Timer.Start();
        }
        return 0;

    case WM_SIZE:
        ClientWidth = LOWORD(lParam);
        ClientHeight = HIWORD(lParam);
        if (m_GraphicsBackend)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (!m_Resizing)
                {
                    OnResize();
                }
            }
        }
        return 0;

    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        m_Timer.Stop();
        return 0;

    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        m_Timer.Start();
        OnResize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_MENUCHAR:
        return MAKELRESULT(0, 1);

    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool D3DApp::InitMainWindow()
{
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = m_hAppInst;
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
    wc.lpszClassName = L"D3DWndClassName";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
    const int width = windowRect.right - windowRect.left;
    const int height = windowRect.bottom - windowRect.top;

    m_hMainWnd = CreateWindow(
        L"D3DWndClassName",
        m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        0,
        0,
        m_hAppInst,
        0);
    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);
    return true;
}

bool D3DApp::InitGraphicsBackend()
{
    m_GraphicsBackend = CreateGraphicsBackend(m_GraphicsBackendType);
    if (!m_GraphicsBackend)
    {
        MessageBox(0, L"The selected graphics backend is not implemented.", 0, 0);
        return false;
    }

    if (!m_GraphicsBackend->Initialize(m_hMainWnd, ClientWidth, ClientHeight, m_Enable4xMsaa))
    {
        const wchar_t* backendName = m_GraphicsBackendType == GraphicsBackendType::DX12 ? L"DirectX 12" : L"DirectX 11";
        std::wstring message = std::wstring(backendName) + L" initialization failed.";
        MessageBox(0, message.c_str(), L"MoonRender", 0);
        return false;
    }

    return true;
}

void D3DApp::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        const float fps = static_cast<float>(frameCnt);
        const float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

bool D3DApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsLight();
    io.FontGlobalScale = 1.0f;

    MoonEnsureDirectory("Builds/Runtime");
    static std::string imguiIniPath = MoonGetAssetPath("Builds/Runtime/imgui.ini");
    io.IniFilename = imguiIniPath.c_str();

    if (!ImGui_ImplWin32_Init(m_hMainWnd))
    {
        return false;
    }

    const std::string fontPath = MoonGetAssetPath("Resources/Fonts/hanyiyingsong45jian.ttf");
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
    io.FontDefault = font;

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    return Graphics().InitializeImGui(m_hMainWnd);
}

void D3DApp::DrawUI()
{
}

IGraphicsBackend& D3DApp::Graphics()
{
    assert(m_GraphicsBackend != nullptr);
    return *m_GraphicsBackend;
}

const IGraphicsBackend& D3DApp::Graphics() const
{
    assert(m_GraphicsBackend != nullptr);
    return *m_GraphicsBackend;
}
