#include "Source/UI/ViewportWindow.h"

#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    ViewportWindow::ViewportWindow() = default;
    ViewportWindow::~ViewportWindow() = default;

    void ViewportWindow::Draw(void* textureId, ViewportInfo& outInfo)
    {
        if (!m_isOpen)
        {
            outInfo = {};
            return;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Scene View", &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();

        outInfo.isOpen = m_isOpen;
        outInfo.focused = ImGui::IsWindowFocused();
        outInfo.hovered = ImGui::IsWindowHovered();

        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
        ImVec2 windowPos = ImGui::GetWindowPos();

        outInfo.posX = windowPos.x + contentMin.x;
        outInfo.posY = windowPos.y + contentMin.y;
        outInfo.width = contentMax.x - contentMin.x;
        outInfo.height = contentMax.y - contentMin.y;

        if (outInfo.width < 1.0f) outInfo.width = 1.0f;
        if (outInfo.height < 1.0f) outInfo.height = 1.0f;

        if (textureId != nullptr)
        {
            ImGui::Image(
                textureId,
                ImVec2(outInfo.width, outInfo.height),
                ImVec2(0.0f, 0.0f),
                ImVec2(1.0f, 1.0f));
        }
        else
        {
            ImGui::Text("No viewport texture");
        }

        ImGui::End();
    }
}
