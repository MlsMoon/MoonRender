#include "Source/UI/Popups/PopupBase.h"

namespace MoonUI
{
    void PopupBase::Open()
    {
        m_shouldOpen = true;
    }

    ImVec2 PopupBase::GetInitialSize() const
    {
        return ImVec2(0.0f, 0.0f);
    }

    void PopupBase::Draw()
    {
        if (m_shouldOpen)
        {
            ImGui::OpenPopup(GetPopupId());
            m_shouldOpen = false;
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        ImVec2 size = GetInitialSize();
        if (size.x > 0.0f && size.y > 0.0f)
        {
            ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
        }

        if (ImGui::BeginPopupModal(GetPopupId(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            DrawContent();
            ImGui::EndPopup();
        }
    }
}
