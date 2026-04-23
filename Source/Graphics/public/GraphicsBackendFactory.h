#pragma once

#include <memory>

#include "GraphicsBackend.h"

std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(GraphicsBackendType backendType);
