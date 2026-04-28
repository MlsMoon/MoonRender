#include "../public/App.h"

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    GraphicsBackendType SelectGraphicsBackend(int argc, char** argv, bool& confirmed)
    {
        confirmed = true;

        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if (arg.find("--gfx=dx12") != std::string::npos)
            {
                return GraphicsBackendType::DX12;
            }
            if (arg.find("--gfx=dx11") != std::string::npos)
            {
                return GraphicsBackendType::DX11;
            }
            if (arg.find("--gfx=metal") != std::string::npos)
            {
                return GraphicsBackendType::Metal;
            }
            if (arg.find("--gfx=cancel") != std::string::npos)
            {
                confirmed = false;
                return GraphicsBackendType::DX11;
            }
        }

#ifdef _WIN32
        return GraphicsBackendType::DX11;
#elif __APPLE__
        return GraphicsBackendType::Metal;
#else
        return GraphicsBackendType::DX11;
#endif
    }
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char** argv)
{
#endif
#if defined(DEBUG) || defined(_DEBUG) && defined(_WIN32)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    freopen("Builds/Runtime/render_log.txt", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    bool confirmed = false;
    const GraphicsBackendType backendType = SelectGraphicsBackend(argc, argv, confirmed);
    if (!confirmed)
    {
        return 0;
    }

    App theApp("MoonRender", 1280, 720, backendType);
    if (!theApp.Init())
    {
        printf("Init failed\n");
        system("pause");
        return 0;
    }

    int ret = theApp.Run();
    printf("Run finished: %d\n", ret);
    system("pause");
    return ret;
}
