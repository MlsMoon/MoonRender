#pragma once

#include <cstdint>

namespace MoonUI
{
    struct ViewportInfo
    {
        float posX = 0.0f;
        float posY = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        bool hovered = false;
        bool focused = false;
        bool isOpen = true;
    };

    class ViewportWindow
    {
    public:
        ViewportWindow();
        ~ViewportWindow();

        void Draw(void* textureId, ViewportInfo& outInfo);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = true;
    };
}
