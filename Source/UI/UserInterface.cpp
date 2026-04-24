#include "UserInterface.h"

#include <string>

#include "Source/Object/MoonObject.h"
#include "Source/Object/TransformComponent.h"

namespace MoonUI
{
    bool UserInterface::DrawMainInterfaceUI(
        const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject)
    {
        DrawMainMenu();
        DrawDockSpace();

        if (showOutlineWindow)
        {
            DrawOutlineView(sceneObjects, selectedObject);
        }

        if (showCameraWindow)
        {
            DrawCameraView();
        }

        if (showOutputWindow)
        {
            DrawOutputLog();
        }

        return true;
    }

    void UserInterface::DrawMainMenu()
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
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("OutlineView", nullptr, &showOutlineWindow);
            ImGui::MenuItem("Camera", nullptr, &showCameraWindow);
            ImGui::MenuItem("OutputLog", nullptr, &showOutputWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void UserInterface::DrawDockSpace()
    {
        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspaceFlags);
    }

    void UserInterface::DrawOutlineView(
        const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(280.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(16.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutlineView", &showOutlineWindow);

        for (const auto& object : sceneObjects)
        {
            if (!object->HasComponent<Object::TransformComponent>())
            {
                continue;
            }

            const bool isSelected = selectedObject == object.get();
            if (ImGui::Selectable(object->GetName().c_str(), isSelected))
            {
                selectedObject = object.get();
            }
        }
        ImGui::End();
    }

    void UserInterface::DrawCameraView()
    {
        ImGui::SetNextWindowSize(ImVec2(320.0f, 140.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(312.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Camera", &showCameraWindow);
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("FOV", &ui_camera_fov, 10.0f, 120.0f, "%.1f deg"))
        {
            EventCenter::EventTrigger("SetCameraFOVValue", ui_camera_fov);
        }
        ImGui::End();
    }

    void UserInterface::DrawOutputLog()
    {
        ImGui::SetNextWindowSize(ImVec2(640.0f, 220.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(312.0f, 512.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutputLog", &showOutputWindow);
        if (log_system == nullptr)
        {
            ImGui::TextDisabled("No log system bound.");
        }
        else
        {
            ImGui::BeginChild("Text Output", ImVec2(0, 0), true);
            const std::string logContent = log_system->GetLogContent();
            ImGui::TextUnformatted(logContent.c_str());
            ImGui::EndChild();
        }
        ImGui::End();
    }

    bool UserInterface::BindLogSystem(Logging::LogSystem* log_system)
    {
        this->log_system = log_system;
        return true;
    }

    UserInterface::UserInterface() = default;

    UserInterface::~UserInterface() = default;
}
