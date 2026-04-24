#pragma once

#include "Source/Logging/public/LogSystem.h"

namespace MoonUI
{
    class OutputLogWindow
    {
    public:
        void Draw();
        void BindLogSystem(Logging::LogSystem* logSystem);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = false;
        Logging::LogSystem* m_logSystem = nullptr;
    };
}
