#include "UserInterface.h"

#include "Source/AppWin/public/GameApp.h"


namespace MoonUI
{
    bool UserInterface::DrawMainInterfaceUI()
    {

        
        // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // 设置alpha分量为0.5，表示半透明
        // ImGui::SetNextWindowSize(ImVec2(GameApp::currentGameApp->ClientWidth +1 , GameApp::currentGameApp->ClientHeight -25));
        // ImGui::SetNextWindowPos(ImVec2(-1,25));
        // const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoFocusOnAppearing;
        // ImGui::Begin("MoonRenderer",nullptr,window_flags);
        // {
        //     // 恢复窗口背景颜色
        //     ImGui::PopStyleColor();
        // }
        // ImGui::End();


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
            ImGui::MenuItem("Camera",nullptr,&showCameraWindow);
            ImGui::MenuItem("OutputLog",nullptr,&showOutputWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
        
        //独立窗口
        if (showCameraWindow)
        {
            ImGui::Begin("Camera",&showCameraWindow);
            if (ImGui::SliderFloat("Camera FOV", &ui_camera_fov, 10.0f, 120.0f))
            {
                EventCenter::EventTrigger("SetCameraFOVValue",ui_camera_fov);
            }
            ImGui::End();
        }


        if (showOutputWindow)
        {
            if (log_system == nullptr)
            {
                log_system = &GameApp::currentGameApp->log_system;
            }
            ImGui::Begin("OutputLog",&showOutputWindow);
            // 创建子窗口，设置滚动条
            ImGui::BeginChild("Text Output", ImVec2(0,0), true);

            ImGui::Text(log_system->GetLogContent().c_str());

            // 结束子窗口
            ImGui::EndChild();
            ImGui::End();
        }

        
        return true;

    }

    bool UserInterface::BindLogSystem(const Logging::LogSystem* log_system)
    {
        return true;
    }

    UserInterface::UserInterface()
    {
    }

    UserInterface::~UserInterface()
    {
    }

}

