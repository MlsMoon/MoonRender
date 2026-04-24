#include "UserInterface.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "Source/Object/CameraComponent.h"
#include "Source/Object/MeshComponent.h"
#include "Source/Object/MoonComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/Object/TransformComponent.h"

namespace MoonUI
{
    namespace
    {
        const char* GetGraphicsBackendDisplayName(GraphicsBackendType graphicsBackendType)
        {
            switch (graphicsBackendType)
            {
            case GraphicsBackendType::DX11:
                return "DirectX 11";
            case GraphicsBackendType::DX12:
                return "DirectX 12";
            default:
                return "Unknown";
            }
        }

        void DrawPropertyControl(Object::ComponentProperty& property)
        {
            const ImGuiSliderFlags sliderFlags = property.clamp ? ImGuiSliderFlags_AlwaysClamp : 0;

            switch (property.type)
            {
            case Object::ComponentPropertyType::Text:
            {
                const std::string value = property.getText ? property.getText() : "";
                ImGui::TextWrapped("%s", value.c_str());
                break;
            }
            case Object::ComponentPropertyType::Float:
            {
                float value = property.getFloat ? property.getFloat() : 0.0f;
                if (property.readOnly)
                {
                    ImGui::Text(property.format, value);
                    break;
                }

                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragFloat("##value", &value, property.speed, property.minValue, property.maxValue, property.format, sliderFlags))
                {
                    if (property.clamp)
                    {
                        value = value < property.minValue ? property.minValue : value;
                        value = value > property.maxValue ? property.maxValue : value;
                    }
                    if (property.setFloat)
                    {
                        property.setFloat(value);
                    }
                }
                break;
            }
            case Object::ComponentPropertyType::Float3:
            {
                DirectX::XMFLOAT3 value = property.getFloat3 ? property.getFloat3() : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
                if (property.readOnly)
                {
                    ImGui::Text(property.format, value.x, value.y, value.z);
                    break;
                }

                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragFloat3("##value", &value.x, property.speed, property.minValue, property.maxValue, property.format, sliderFlags))
                {
                    if (property.clamp)
                    {
                        value.x = value.x < property.minValue ? property.minValue : value.x;
                        value.x = value.x > property.maxValue ? property.maxValue : value.x;
                        value.y = value.y < property.minValue ? property.minValue : value.y;
                        value.y = value.y > property.maxValue ? property.maxValue : value.y;
                        value.z = value.z < property.minValue ? property.minValue : value.z;
                        value.z = value.z > property.maxValue ? property.maxValue : value.z;
                    }
                    if (property.setFloat3)
                    {
                        property.setFloat3(value);
                    }
                }
                break;
            }
            case Object::ComponentPropertyType::Float4:
            {
                DirectX::XMFLOAT4 value = property.getFloat4 ? property.getFloat4() : DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
                if (property.readOnly)
                {
                    ImGui::Text(property.format, value.x, value.y, value.z, value.w);
                    break;
                }

                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::DragFloat4("##value", &value.x, property.speed, property.minValue, property.maxValue, property.format, sliderFlags))
                {
                    if (property.clamp)
                    {
                        value.x = value.x < property.minValue ? property.minValue : value.x;
                        value.x = value.x > property.maxValue ? property.maxValue : value.x;
                        value.y = value.y < property.minValue ? property.minValue : value.y;
                        value.y = value.y > property.maxValue ? property.maxValue : value.y;
                        value.z = value.z < property.minValue ? property.minValue : value.z;
                        value.z = value.z > property.maxValue ? property.maxValue : value.z;
                        value.w = value.w < property.minValue ? property.minValue : value.w;
                        value.w = value.w > property.maxValue ? property.maxValue : value.w;
                    }
                    if (property.setFloat4)
                    {
                        property.setFloat4(value);
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }

    bool UserInterface::DrawMainInterfaceUI(
        std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject,
        GraphicsBackendType graphicsBackendType)
    {
        DrawMainMenu();
        DrawDockSpace();

        if (showOutlineWindow)
        {
            DrawOutlineView(sceneObjects, selectedObject);
        }

        if (showInspectorWindow)
        {
            DrawInspectorView(selectedObject);
        }

        if (showGlobalSettingWindow)
        {
            DrawGlobalSettingView(graphicsBackendType);
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
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("OutlineView", nullptr, &showOutlineWindow);
            ImGui::MenuItem("Inspector", nullptr, &showInspectorWindow);
            ImGui::MenuItem("Global Setting", nullptr, &showGlobalSettingWindow);
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
        std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(280.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(16.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutlineView", &showOutlineWindow);

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

    void UserInterface::DrawInspectorView(Object::MoonObject* selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(984.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inspector", &showInspectorWindow);

        if (selectedObject == nullptr)
        {
            ImGui::TextDisabled("No object selected.");
            ImGui::End();
            return;
        }

        char nameBuffer[128] = {};
        const std::string& currentDisplayName = selectedObject->GetDisplayName();
        const size_t copyLen = (std::min)(currentDisplayName.size(), sizeof(nameBuffer) - 1);
        std::memcpy(nameBuffer, currentDisplayName.c_str(), copyLen);
        nameBuffer[copyLen] = '\0';

        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            selectedObject->SetDisplayName(nameBuffer);
        }
        ImGui::Separator();

        int componentIndex = 0;
        for (const auto& component : selectedObject->GetComponents())
        {
            component->Normalize();
            ImGui::PushID(componentIndex);
            if (ImGui::CollapsingHeader(component->GetDisplayName(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                std::vector<Object::ComponentProperty> properties = component->GetProperties();
                if (ImGui::BeginTable("ComponentProperties", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
                {
                    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 118.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    int propertyIndex = 0;
                    for (Object::ComponentProperty& property : properties)
                    {
                        ImGui::PushID(propertyIndex);
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted(property.name);

                        ImGui::TableSetColumnIndex(1);
                        DrawPropertyControl(property);
                        ImGui::PopID();
                        ++propertyIndex;
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::PopID();
            ++componentIndex;
        }

        ImGui::End();
    }

    void UserInterface::DrawGlobalSettingView(GraphicsBackendType graphicsBackendType)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 140.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(312.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Global Setting", &showGlobalSettingWindow);

        if (ImGui::BeginTable("GlobalSettingProperties", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 128.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Graphics API");

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(GetGraphicsBackendDisplayName(graphicsBackendType));

            ImGui::EndTable();
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
