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
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
            }

            if (ImGui::MenuItem("Save"))
            {
            }

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

        if (showOutlineWindow)
        {
            DrawOutlineView(sceneObjects, selectedObject);
        }

        if (showCameraWindow)
        {
            ImGui::Begin("Camera", &showCameraWindow);
            if (ImGui::SliderFloat("Camera FOV", &ui_camera_fov, 10.0f, 120.0f))
            {
                EventCenter::EventTrigger("SetCameraFOVValue", ui_camera_fov);
            }
            ImGui::End();
        }

        if (showOutputWindow)
        {
            ImGui::Begin("OutputLog", &showOutputWindow);
            if (log_system == nullptr)
            {
                ImGui::Text("No log system bound.");
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

        return true;
    }

    void UserInterface::DrawOutlineView(
        const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject)
    {
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

    bool UserInterface::BindLogSystem(Logging::LogSystem* log_system)
    {
        this->log_system = log_system;
        return true;
    }

    UserInterface::UserInterface() = default;

    UserInterface::~UserInterface() = default;
}
