#include "Source/Graphics/public/GraphicsBackendFactory.h"

#ifdef _WIN32
std::unique_ptr<IGraphicsBackend> CreateDx11Backend();
std::unique_ptr<IGraphicsBackend> CreateDx12Backend();
#endif
#ifdef __APPLE__
std::unique_ptr<IGraphicsBackend> CreateMetalBackend();
#endif

std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(GraphicsBackendType backendType)
{
    switch (backendType)
    {
#ifdef _WIN32
    case GraphicsBackendType::DX11:
        return CreateDx11Backend();
    case GraphicsBackendType::DX12:
        return CreateDx12Backend();
#endif
#ifdef __APPLE__
    case GraphicsBackendType::Metal:
        return CreateMetalBackend();
#endif
    default:
        return nullptr;
    }
}
