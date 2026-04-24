#pragma once

#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    class PopupBase
    {
    public:
        virtual ~PopupBase() = default;

        void Open();
        void Draw();

    protected:
        virtual const char* GetPopupId() const = 0;
        virtual void DrawContent() = 0;
        virtual ImVec2 GetInitialSize() const;

    private:
        bool m_shouldOpen = false;
    };
}
