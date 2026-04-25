#include "Source/UI/OutlineWindow.h"

#include <algorithm>
#include <string>

#include "Source/Object/CameraComponent.h"
#include "Source/Object/MeshComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/Object/Scene.h"
#include "Source/Object/TransformComponent.h"
#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    void OutlineWindow::Draw(Object::Scene& scene, Object::MoonObject*& selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(280.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(16.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutlineView", &m_isOpen);

        Object::MoonObject* objectToDelete = nullptr;

        for (Object::MoonObject* root : scene.GetRootObjects())
        {
            if (root != nullptr && root->HasComponent<Object::TransformComponent>())
            {
                DrawObjectTree(root, selectedObject, scene);
            }
        }

        if (objectToDelete != nullptr)
        {
            if (selectedObject == objectToDelete)
            {
                selectedObject = nullptr;
            }
            scene.DestroyObject(objectToDelete);
        }

        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Object"))
            {
                static int emptyObjectCounter = 1;
                const std::string name = "EmptyObject_" + std::to_string(emptyObjectCounter);
                auto* newObject = scene.SpawnObject(name);
                newObject->SetDisplayName("Empty Object " + std::to_string(emptyObjectCounter));
                newObject->AddComponent<Object::TransformComponent>();
                selectedObject = newObject;
                ++emptyObjectCounter;
            }
            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Sphere"))
                {
                    static int sphereObjectCounter = 1;
                    const std::string name = "Sphere_" + std::to_string(sphereObjectCounter);
                    auto* newObject = scene.SpawnObject(name);
                    newObject->SetDisplayName("Sphere " + std::to_string(sphereObjectCounter));
                    newObject->AddComponent<Object::TransformComponent>();
                    newObject->AddComponent<Object::MeshComponent>("Resources/Models/sphere.obj", ResourcesProcess::OBJ);
                    selectedObject = newObject;
                    ++sphereObjectCounter;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        m_deleteCameraWarningPopup.Draw();

        ImGui::End();
    }

    void OutlineWindow::DrawObjectTree(Object::MoonObject* object, Object::MoonObject*& selectedObject, Object::Scene& scene)
    {
        if (object == nullptr)
        {
            return;
        }

        ImGui::PushID(object);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (object->GetChildren().empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (selectedObject == object)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (selectedObject == object)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.36f, 0.62f, 0.87f, 0.50f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.46f, 0.74f, 1.00f, 0.55f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.13f, 0.15f, 0.18f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.18f, 0.23f, 0.29f, 1.00f));
        }

        const bool nodeOpen = ImGui::TreeNodeEx(object->GetDisplayName().c_str(), flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            selectedObject = object;
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::BeginMenu("Create Child"))
            {
                if (ImGui::MenuItem("Empty Object"))
                {
                    static int childEmptyCounter = 1;
                    auto* newObject = scene.SpawnObject("ChildEmpty_" + std::to_string(childEmptyCounter));
                    newObject->SetDisplayName("Child Empty " + std::to_string(childEmptyCounter));
                    newObject->AddComponent<Object::TransformComponent>();
                    newObject->SetParent(object);
                    selectedObject = newObject;
                    ++childEmptyCounter;
                }
                if (ImGui::BeginMenu("Mesh"))
                {
                    if (ImGui::MenuItem("Sphere"))
                    {
                        static int childSphereCounter = 1;
                        auto* newObject = scene.SpawnObject("ChildSphere_" + std::to_string(childSphereCounter));
                        newObject->SetDisplayName("Child Sphere " + std::to_string(childSphereCounter));
                        newObject->AddComponent<Object::TransformComponent>();
                        newObject->AddComponent<Object::MeshComponent>("Resources/Models/sphere.obj", ResourcesProcess::OBJ);
                        newObject->SetParent(object);
                        selectedObject = newObject;
                        ++childSphereCounter;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Object"))
            {
                if (object->HasComponent<Object::CameraComponent>())
                {
                    int cameraCount = 0;
                    for (const auto& obj : scene.GetObjects())
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
                        if (selectedObject == object)
                        {
                            selectedObject = nullptr;
                        }
                        scene.DestroyObject(object);
                    }
                }
                else
                {
                    if (selectedObject == object)
                    {
                        selectedObject = nullptr;
                    }
                    scene.DestroyObject(object);
                }
            }
            ImGui::EndPopup();
        }

        if (nodeOpen)
        {
            for (Object::MoonObject* child : object->GetChildren())
            {
                if (child != nullptr && child->HasComponent<Object::TransformComponent>())
                {
                    DrawObjectTree(child, selectedObject, scene);
                }
            }
            ImGui::TreePop();
        }

        ImGui::PopStyleColor(2);
        ImGui::PopID();
    }
}
