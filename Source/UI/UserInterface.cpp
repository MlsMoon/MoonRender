#include "UserInterface.h"



namespace MoonUI
{
    bool UserInterface::DrawMainInterfaceUI()
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
        if (ImGui::SliderFloat("Camera FOV", &ui_camera_fov, 10.0f, 120.0f))
        {
            EventCenter::EventTrigger("SetCameraFOVValue",ui_camera_fov);
        }
        ImGui::End();



    
        return true;
    }

    UserInterface::UserInterface()
    {
    }

    UserInterface::~UserInterface()
    {
    }

}

