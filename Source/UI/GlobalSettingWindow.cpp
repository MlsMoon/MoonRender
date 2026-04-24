#include "Source/UI/GlobalSettingWindow.h"

#include "Source/ThirdParty/ImGui/imgui.h"

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
    }

    void GlobalSettingWindow::Draw(GraphicsBackendType graphicsBackendType)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 140.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(312.0f, 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Global Setting", &m_isOpen);

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
}
