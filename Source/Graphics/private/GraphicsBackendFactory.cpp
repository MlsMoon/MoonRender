#include "Source/Graphics/public/GraphicsBackendFactory.h"

std::unique_ptr<IGraphicsBackend> CreateDx11Backend();
std::unique_ptr<IGraphicsBackend> CreateDx12Backend();

std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(GraphicsBackendType backendType)
{
    switch (backendType)
    {
    case GraphicsBackendType::DX11:
        return CreateDx11Backend();
    case GraphicsBackendType::DX12:
        return CreateDx12Backend();
    default:
        return nullptr;
    }
}
