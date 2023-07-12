#include "UI.h"


bool UI::DrawMainInterfaceUI()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open"))
        {
            // 处理 "Open" 被点击的逻辑
        }

        if (ImGui::MenuItem("Save"))
        {
            // 处理 "Save" 被点击的逻辑
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Windows"))
    {
        if (ImGui::MenuItem("Test"))
        {
            // 处理 "Open" 被点击的逻辑
        }

        //if (ImGui::MenuItem("Save"))
        //{
        //    // 处理 "Save" 被点击的逻辑
        //}
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    return true;
}
