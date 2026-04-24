#include "Source/UI/Popups/DeleteCameraWarningPopup.h"

namespace MoonUI
{
    const char* DeleteCameraWarningPopup::GetPopupId() const
    {
        return "DeleteCameraWarningPopup";
    }

    void DeleteCameraWarningPopup::DrawContent()
    {
        ImGui::TextUnformatted("Cannot delete the last camera in the scene.");
        ImGui::Spacing();
        ImGui::TextUnformatted("At least one camera is required for rendering.");
        ImGui::Spacing();

        const float buttonWidth = 120.0f;
        const float windowWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
    }
}
