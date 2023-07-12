#include "UserInterface.h"


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
        if (ImGui::MenuItem("Camera"))
        {
            // 处理 "Camera" 被点击的逻辑
        }

        //if (ImGui::MenuItem("Save"))
        //{
        //    // 处理 "Save" 被点击的逻辑
        //}
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    
    ImGui::Begin("Camera");
    static float ui_camera_fov_value = 0;
    if (ImGui::SliderFloat("Camera FOV", &ui_camera_fov_value, 0.0f, 1.0f))
    {
        
    }
    ImGui::End();



    
    return true;
}
