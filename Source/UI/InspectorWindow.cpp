#include "Source/UI/InspectorWindow.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "Source/Object/MoonComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    namespace
    {
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

    void InspectorWindow::Draw(Object::MoonObject* selectedObject)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 520.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(984.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inspector", &m_isOpen);

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
}
