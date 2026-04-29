#include "Source/UI/MainMenuBar.h"

#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    void MainMenuBar::Draw(bool* showOutline, bool* showInspector, bool* showGlobalSetting, bool* showOutputLog, bool* showViewport)
    {
        if (!ImGui::BeginMainMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open");
            ImGui::MenuItem("Save");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Scene View", nullptr, showViewport);
            ImGui::MenuItem("OutlineView", nullptr, showOutline);
            ImGui::MenuItem("Inspector", nullptr, showInspector);
            ImGui::MenuItem("Global Setting", nullptr, showGlobalSetting);
            ImGui::MenuItem("OutputLog", nullptr, showOutputLog);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
