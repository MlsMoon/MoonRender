#include "Source/UI/OutlineWindow.h"

#include <algorithm>
#include <string>
#include <string>

#include "Source/Object/CameraComponent.h"
#include "Source/Object/MeshComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/Object/TransformComponent.h"
#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    void OutlineWindow::Draw(std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects, Object::MoonObject*& selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(280.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(16.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutlineView", &m_isOpen);

        Object::MoonObject* objectToDelete = nullptr;

        for (const auto& object : sceneObjects)
        {
            if (!object->HasComponent<Object::TransformComponent>())
            {
                continue;
            }

            ImGui::PushID(object.get());
            const bool isSelected = selectedObject == object.get();
            if (ImGui::Selectable(object->GetDisplayName().c_str(), isSelected))
            {
                selectedObject = object.get();
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete Object"))
                {
                    if (object->HasComponent<Object::CameraComponent>())
                    {
                        int cameraCount = 0;
                        for (const auto& obj : sceneObjects)
                        {
                            if (obj->HasComponent<Object::CameraComponent>())
                            {
                                ++cameraCount;
                            }
                        }
                        if (cameraCount <= 1)
                        {
                            m_deleteCameraWarningPopup.Open();
                        }
                        else
                        {
                            objectToDelete = object.get();
                        }
                    }
                    else
                    {
                        objectToDelete = object.get();
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }

        if (objectToDelete != nullptr)
        {
            if (selectedObject == objectToDelete)
            {
                selectedObject = nullptr;
            }
            sceneObjects.erase(
                std::remove_if(sceneObjects.begin(), sceneObjects.end(),
                    [objectToDelete](const auto& obj) { return obj.get() == objectToDelete; }),
                sceneObjects.end());
        }

        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Object"))
            {
                static int emptyObjectCounter = 1;
                const std::string internalName = "EmptyObject_" + std::to_string(emptyObjectCounter);
                const std::string displayName = "Empty Object " + std::to_string(emptyObjectCounter);
                auto newObject = std::make_unique<Object::MoonObject>(internalName);
                newObject->SetDisplayName(displayName);
                newObject->AddComponent<Object::TransformComponent>();
                selectedObject = newObject.get();
                sceneObjects.push_back(std::move(newObject));
                ++emptyObjectCounter;
            }
            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Sphere"))
                {
                    static int sphereObjectCounter = 1;
                    const std::string internalName = "Sphere_" + std::to_string(sphereObjectCounter);
                    const std::string displayName = "Sphere " + std::to_string(sphereObjectCounter);
                    auto newObject = std::make_unique<Object::MoonObject>(internalName);
                    newObject->SetDisplayName(displayName);
                    newObject->AddComponent<Object::TransformComponent>();
                    newObject->AddComponent<Object::MeshComponent>("Resources/Models/sphere.obj", ResourcesProcess::OBJ);
                    selectedObject = newObject.get();
                    sceneObjects.push_back(std::move(newObject));
                    ++sphereObjectCounter;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        m_deleteCameraWarningPopup.Draw();

        ImGui::End();
    }
}
