#include "../public/D3DApp.h"



#pragma warning(disable: 6031)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern "C"
{
    // 在具有多显卡的硬件设备中，优先使用NVIDIA或AMD的显卡运行
    // 需要在.exe中使用
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    D3DApp* g_pd3dApp = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before m_hMainWnd is valid.
    return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : m_hAppInst(hInstance),
    m_MainWndCaption(windowName),
    ClientWidth(initWidth),
    ClientHeight(initHeight),
    m_hMainWnd(nullptr),
    m_AppPaused(false),
    m_Minimized(false),
    m_Maximized(false),
    m_Resizing(false),
    m_Enable4xMsaa(true),
    m_4xMsaaQuality(0),
    m_pd3dDevice(nullptr),
    m_pd3dImmediateContext(nullptr),
    m_pSwapChain(nullptr),
    m_pDepthStencilBuffer(nullptr),
    m_pRenderTargetView(nullptr),
    m_pDepthStencilView(nullptr)
{
    ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));


    // 让一个全局指针获取这个类，这样我们就可以在Windows消息处理的回调函数
    // 让这个类调用内部的回调函数了
    g_pd3dApp = this;
}

D3DApp::~D3DApp()
{
    // 恢复所有默认设定
    if (m_pd3dImmediateContext)
        m_pd3dImmediateContext->ClearState();
}

HINSTANCE D3DApp::AppInst()const
{
    return m_hAppInst;
}

HWND D3DApp::MainWnd()const
{
    return m_hMainWnd;
}

float D3DApp::AspectRatio()const
{
    return static_cast<float>(ClientWidth) / ClientHeight;
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

                //UI绘制 - 新的Frame
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                //绘制界面逻辑
                DrawUI();

                //更新场景 /绘制场景
                UpdateScene(m_Timer.DeltaTime());
                DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

/// <summary>
/// 初始化函数
/// </summary>
/// <returns></returns>
bool D3DApp::Init()
{
    //第一步 初始化窗口
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    if (!InitImGui())
        return false;

    return true;
}

/// <summary>
/// 每次在修改窗口大小的时候
/// 都需要把资源释放掉
/// 然后重新初始化一遍
/// </summary>
void D3DApp::OnResize()
{
    assert(m_pd3dImmediateContext);
    assert(m_pd3dDevice);
    assert(m_pSwapChain);

    if (m_pd3dDevice1 != nullptr)
    {
        assert(m_pd3dImmediateContext1);
        assert(m_pd3dDevice1);
        assert(m_pSwapChain1);
    }

    // 释放渲染管线输出用到的相关资源
    m_pRenderTargetView.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencilBuffer.Reset();

    // 重设交换链并且重新创建渲染目标视图
    // 在D3D初始化的函数中，会创建一个DXGI交换链
    // 创建的交换链自带了一个后背缓冲区，通过GetBuffer来获取
    ComPtr<ID3D11Texture2D> backBuffer;
    HR(m_pSwapChain->ResizeBuffers(1, ClientWidth, ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    //HRESULT IDXGISwapChain::GetBuffer(
    //    UINT Buffer,        // [In]缓冲区索引号，从0到BufferCount - 1
    //    REFIID riid,        // [In]缓冲区的接口类型ID
    //    void** ppSurface);  // [Out]获取到的缓冲区
    // ** 第二个参数，获取的是类型的UUID
    HR(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));

    //创建RenderTargetView，可以看到，创建的同时，我们将交换链中的后备缓冲区，绑定到RenderTargetView上（第一个参数）
    //HRESULT ID3D11Device::CreateRenderTargetView(
    //    ID3D11Resource * pResource,                      // [In]待绑定到渲染目标视图的资源
    //    const D3D11_RENDER_TARGET_VIEW_DESC * pDesc,     // [In]忽略
    //    ID3D11RenderTargetView * *ppRTView);             // [Out]获取渲染目标视图
    HR(m_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf()));

    // 设置调试对象名
    D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");

    backBuffer.Reset();

    //接下来创建 Depth / Stencil Buffer
    //    和RTview 不一样，这次没有 现成的Texture2D 资源，需要手动创建一个 2d纹理资源
    //    通过D3D设备可以新建一个2D纹理，但在此之前我们需要先描述该缓冲区的信息：

    //有以下可以用来描述一个Texture2d的字段
    //    typedef struct D3D11_TEXTURE2D_DESC
    //{
    //    UINT Width;         // 缓冲区宽度
    //    UINT Height;        // 缓冲区高度
    //    UINT MipLevels;     // Mip等级
    //    UINT ArraySize;     // 纹理数组中的纹理数量，默认1
    //    DXGI_FORMAT Format; // 缓冲区数据格式
    //    DXGI_SAMPLE_DESC SampleDesc;    // MSAA采样描述
    //    D3D11_USAGE Usage;  // 数据的CPU/GPU访问权限
    //    UINT BindFlags;     // 使用D3D11_BIND_FLAG枚举来决定该数据的使用类型
    //    UINT CPUAccessFlags;    // 使用D3D11_CPU_ACCESS_FLAG枚举来决定CPU访问权限
    //    UINT MiscFlags;     // 使用D3D11_RESOURCE_MISC_FLAG枚举，这里默认0
    //}

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = ClientWidth;
    depthStencilDesc.Height = ClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 要使用 4X MSAA? --需要给交换链设置MASS参数
    if (m_Enable4xMsaa)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    }
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }



    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    // 创建深度缓冲区以及深度模板视图
    //HRESULT ID3D11Device::CreateTexture2D(
    //    const D3D11_TEXTURE2D_DESC * pDesc,          // [In] 2D纹理描述信息
    //    const D3D11_SUBRESOURCE_DATA * pInitialData, // [In] 用于初始化的资源
    //    ID3D11Texture2D * *ppTexture2D);             // [Out] 获取到的2D纹理
    HR(m_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, m_pDepthStencilBuffer.GetAddressOf()));

    //有了深度 / 模板缓冲区后，就可以通过ID3D11Device::CreateDepthStencilView方法将创建好的2D纹理绑定到新建的深度 / 模板视图：

    //    HRESULT ID3D11Device::CreateDepthStencilView(
    //        ID3D11Resource * pResource,                      // [In] 需要绑定的资源
    //        const D3D11_DEPTH_STENCIL_VIEW_DESC * pDesc,     // [In] 深度缓冲区描述，这里忽略
    //        ID3D11DepthStencilView * *ppDepthStencilView);   // [Out] 获取到的深度/模板视图
    // 
    HR(m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDepthStencilView.GetAddressOf()));

    //此时我们资源都创建完毕了，需要将两个View绑定到管线去
    // 将渲染目标视图和深度/模板缓冲区结合到管线
    //void ID3D11DeviceContext::OMSetRenderTargets(
    //    UINT NumViews,                                      // [In] 视图数目
    //    ID3D11RenderTargetView* const* ppRenderTargetViews, // [In] 渲染目标视图数组（多RT是很常见的）
    //    ID3D11DepthStencilView * pDepthStencilView) = 0;     // [In] 深度/模板视图
    m_pd3dImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

    // 设置视口变换
    //最终我们还需要决定将整个视图输出到窗口特定的范围。我们需要使用D3D11_VIEWPORT来设置视口

    //    typedef struct D3D11_VIEWPORT
    //{
    //    FLOAT TopLeftX;     // 屏幕左上角起始位置X
    //    FLOAT TopLeftY;     // 屏幕左上角起始位置Y
    //    FLOAT Width;        // 宽度
    //    FLOAT Height;       // 高度
    //    FLOAT MinDepth;     // 最小深度，必须为0.0f
    //    FLOAT MaxDepth;     // 最大深度，必须为1.0f
    //}     D3D11_VIEWPORT;
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(ClientWidth);
    m_ScreenViewport.Height = static_cast<float>(ClientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_pd3dImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}



/// <summary>
/// 这是一个处理消息的函数
/// </summary>
/// <param name="hwnd"></param>
/// <param name="msg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(m_hMainWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
        // We pause the game when the window is deactivated and unpause it 
        // when it becomes active.  
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

        // WM_SIZE is sent when the user resizes the window.  
    case WM_SIZE:
        // Save the new client area dimensions.
        ClientWidth = LOWORD(lParam);
        ClientHeight = HIWORD(lParam);
        if (m_pd3dDevice)
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

                // Restoring from minimized state?
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }

                // Restoring from maximized state?
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (m_Resizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        m_Timer.Stop();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        m_Timer.Start();
        OnResize();
        return 0;

        // WM_DESTROY is sent when the window is being destroyed.
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses 
        // a key that does not correspond to any mnemonic or accelerator key. 
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        return 0;
    case WM_MOUSEMOVE:
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool D3DApp::InitMainWindow()
{
    //首先创建一个 WNDCLASS 结构体 wc，并初始化其中的各个成员变量。
    //其中，lpfnWndProc 成员变量指向了一个名为 MainWndProc 的静态函数，
    //这个函数是窗口的消息处理函数，用于处理 Windows 消息。
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    //注册窗口类，注册失败后弹出一个对话框
    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, ClientWidth, ClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);

    return true;
}

bool D3DApp::InitDirect3D()
{
    HRESULT hr = S_OK;

    // 创建D3D设备 和 D3D设备上下文
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    // 驱动类型数组
    //
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    // 特性等级数组
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    D3D_DRIVER_TYPE d3dDriverType;

    //查询当前机器有多少显卡
    ComPtr<IDXGIFactory> dxgiFactory = nullptr;
    void** dxgiFactoryPtr = reinterpret_cast<void**>(dxgiFactory.GetAddressOf());

    // 创建DXGI工厂对象
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), dxgiFactoryPtr);
    if (SUCCEEDED(hr))
    {
        std::cout << "IDXGIFactory created successfully." << std::endl;

        // 使用IDXGIFactory对象进行其他操作...
        // 枚举显示适配器
        IDXGIAdapter* dxgiAdapter = nullptr;
        for (UINT i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC adapterDesc;
            hr = dxgiAdapter->GetDesc(&adapterDesc);
            if (SUCCEEDED(hr))
            {
                //
            }
            dxgiAdapter->Release();
            dxgiAdapter = nullptr;
        }
    }
    else
    {
        std::cout << "Failed to create IDXGIFactory: " << std::hex << hr << std::endl;
    }

    //这里遍历的是驱动类型
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        d3dDriverType = driverTypes[driverTypeIndex];
        //Create Device函数 在这里创建了D3D设备
        /*  参数说明：
            IDXGIAdapter* pAdapter,             // [In_Opt]适配器
            D3D_DRIVER_TYPE DriverType,         // [In]驱动类型
            HMODULE Software,                   // [In_Opt]若上面为D3D_DRIVER_TYPE_SOFTWARE则这里需要提供程序模块
            UINT Flags,                         // [In]使用D3D11_CREATE_DEVICE_FLAG枚举类型
            D3D_FEATURE_LEVEL* pFeatureLevels,  // [In_Opt]若为nullptr则为默认特性等级，否则需要提供特性等级数组
            UINT FeatureLevels,                 // [In]特性等级数组的元素数目
            UINT SDKVersion,                    // [In]SDK版本，默认D3D11_SDK_VERSION
            ID3D11Device** ppDevice,            // [Out_Opt]输出D3D设备
            D3D_FEATURE_LEVEL* pFeatureLevel,   // [Out_Opt]输出当前应用D3D特性等级
            ID3D11DeviceContext** ppImmediateContext ); //[Out_Opt]输出D3D设备上下文
        */
        hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());

        if (hr == E_INVALIDARG)
        {
            // Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
            hr = D3D11CreateDevice(nullptr, d3dDriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
        }

        //如果这里成功创建了，就结束
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
    {
        MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
        return false;
    }

    // 检测是否支持特性等级11.0或11.1
    if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1)
    {
        MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
        return false;
    }

    // 检测 MSAA支持的质量等级
    m_pd3dDevice->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
    assert(m_4xMsaaQuality > 0);




    ComPtr<IDXGIDevice> dxgiDevice = nullptr;
    ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
    ComPtr<IDXGIFactory1> dxgiFactory1 = nullptr;   // D3D11.0(包含DXGI1.1)的接口类
    ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;   // D3D11.1(包含DXGI1.2)特有的接口类

    // 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
    // "IDXGIFactory::CreateSwapChain: This function is being called with a device from a different IDXGIFactory."
    HR(m_pd3dDevice.As(&dxgiDevice));
    HR(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));
    HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf())));

    // 查看该对象是否包含IDXGIFactory2接口
    hr = dxgiFactory1.As(&dxgiFactory2);
    // 如果包含，则说明支持D3D11.1
    if (dxgiFactory2 != nullptr)
    {
        HR(m_pd3dDevice.As(&m_pd3dDevice1));
        HR(m_pd3dImmediateContext.As(&m_pd3dImmediateContext1));
        // 填充各种结构体用以描述交换链
        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = ClientWidth;
        sd.Height = ClientHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        // 是否开启4倍多重采样？
        if (m_Enable4xMsaa)
        {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
        }
        else
        {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
        fd.RefreshRate.Numerator = 60;
        fd.RefreshRate.Denominator = 1;
        fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fd.Windowed = TRUE;
        // 为当前窗口创建交换链
        HR(dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), m_hMainWnd, &sd, &fd, nullptr, m_pSwapChain1.GetAddressOf()));
        HR(m_pSwapChain1.As(&m_pSwapChain));
    }
    else
    {
        // 填充DXGI_SWAP_CHAIN_DESC用以描述交换链
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = ClientWidth;
        sd.BufferDesc.Height = ClientHeight;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        // 是否开启4倍多重采样？
        if (m_Enable4xMsaa)
        {
            sd.SampleDesc.Count = 4;
            sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
        }
        else
        {
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
        }
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.OutputWindow = m_hMainWnd;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;
        HR(dxgiFactory1->CreateSwapChain(m_pd3dDevice.Get(), &sd, m_pSwapChain.GetAddressOf()));
    }



    // 可以禁止alt+enter全屏
    dxgiFactory1->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

    // 设置调试对象名
    D3D11SetDebugObjectName(m_pd3dImmediateContext.Get(), "ImmediateContext");
    DXGISetDebugObjectName(m_pSwapChain.Get(), "SwapChain");

    // 每当窗口被重新调整大小的时候，都需要调用这个OnResize函数。现在调用
    // 以避免代码重复
    OnResize();

    return true;
}

void D3DApp::CalculateFrameStats()
{
    // 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

bool D3DApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsLight();
    ImGui::GetIO().FontGlobalScale = 1.0;

    // 设置平台/渲染器后端
    ImGui_ImplWin32_Init(m_hMainWnd);
    ImGui_ImplDX11_Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

    //设置字体
    ImFont* font = io.Fonts->AddFontFromFileTTF("Resources/Fonts/hanyiyingsong45jian.ttf", 16.0f);
    io.FontDefault = font;

    return true;

}

void D3DApp::DrawUI()
{
}