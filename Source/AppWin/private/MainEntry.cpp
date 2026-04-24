#include "../public/App.h"

#include <string>

namespace
{
    constexpr int kBackendButtonDx11 = 1001;
    constexpr int kBackendButtonDx12 = 1002;
    constexpr int kBackendButtonCancel = 1003;
    constexpr wchar_t kSelectorWindowClass[] = L"MoonRenderBackendSelector";

    struct BackendDialogState
    {
        GraphicsBackendType backendType = GraphicsBackendType::DX11;
        bool confirmed = false;
    };

    LRESULT CALLBACK BackendSelectorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
            return TRUE;
        }

        BackendDialogState* state = reinterpret_cast<BackendDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (msg)
        {
        case WM_CREATE:
            CreateWindowW(L"STATIC", L"Choose the graphics API for this session:", WS_CHILD | WS_VISIBLE,
                18, 18, 280, 20, hwnd, nullptr, nullptr, nullptr);
            CreateWindowW(L"STATIC", L"You can add Vulkan or other backends here later.", WS_CHILD | WS_VISIBLE,
                18, 42, 300, 18, hwnd, nullptr, nullptr, nullptr);

            CreateWindowW(L"BUTTON", L"DirectX 11", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                18, 78, 92, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBackendButtonDx11)), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"DirectX 12", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                122, 78, 92, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBackendButtonDx12)), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                226, 78, 92, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kBackendButtonCancel)), nullptr, nullptr);
            return 0;

        case WM_COMMAND:
            if (state == nullptr)
            {
                return 0;
            }

            switch (LOWORD(wParam))
            {
            case kBackendButtonDx11:
                state->backendType = GraphicsBackendType::DX11;
                state->confirmed = true;
                DestroyWindow(hwnd);
                return 0;
            case kBackendButtonDx12:
                state->backendType = GraphicsBackendType::DX12;
                state->confirmed = true;
                DestroyWindow(hwnd);
                return 0;
            case kBackendButtonCancel:
                state->confirmed = false;
                DestroyWindow(hwnd);
                return 0;
            default:
                break;
            }
            break;

        case WM_CLOSE:
            if (state != nullptr)
            {
                state->confirmed = false;
            }
            DestroyWindow(hwnd);
            return 0;

        default:
            break;
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    bool ShowGraphicsBackendDialog(HINSTANCE hInstance, GraphicsBackendType& backendType)
    {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = BackendSelectorWndProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wc.lpszClassName = kSelectorWindowClass;

        RegisterClassW(&wc);

        BackendDialogState state = {};
        HWND hwnd = CreateWindowExW(
            WS_EX_DLGMODALFRAME,
            kSelectorWindowClass,
            L"MoonRender Graphics API",
            WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            352,
            162,
            nullptr,
            nullptr,
            hInstance,
            &state);
        if (hwnd == nullptr)
        {
            return false;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = {};
        while (IsWindow(hwnd) && GetMessageW(&msg, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!state.confirmed)
        {
            return false;
        }

        backendType = state.backendType;
        return true;
    }

    GraphicsBackendType SelectGraphicsBackend(HINSTANCE hInstance, LPSTR cmdLine, bool& confirmed)
    {
        const std::string commandLine = cmdLine != nullptr ? cmdLine : "";
        if (commandLine.find("--gfx=dx12") != std::string::npos)
        {
            confirmed = true;
            return GraphicsBackendType::DX12;
        }
        if (commandLine.find("--gfx=dx11") != std::string::npos)
        {
            confirmed = true;
            return GraphicsBackendType::DX11;
        }

        GraphicsBackendType backendType = GraphicsBackendType::DX11;
        confirmed = ShowGraphicsBackendDialog(hInstance, backendType);
        return backendType;
    }
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE prevInstance,
    _In_ LPSTR cmdLine,
    _In_ int showCmd)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(showCmd);

#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const std::string commandLine = cmdLine != nullptr ? cmdLine : "";
    if (commandLine.find("--gfx=cancel") != std::string::npos)
    {
        return 0;
    }

    bool confirmed = false;
    const GraphicsBackendType backendType = SelectGraphicsBackend(hInstance, cmdLine, confirmed);
    if (!confirmed)
    {
        return 0;
    }

    App theApp(hInstance, L"MoonRender", 1280, 720, backendType);
    if (!theApp.Init())
    {
        return 0;
    }

    return theApp.Run();
}
