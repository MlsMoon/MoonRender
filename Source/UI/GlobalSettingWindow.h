#pragma once

#include "Source/Graphics/public/GraphicsTypes.h"

namespace MoonUI
{
    class GlobalSettingWindow
    {
    public:
        void Draw(GraphicsBackendType graphicsBackendType);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = true;
    };
}
