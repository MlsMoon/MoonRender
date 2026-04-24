#include "Source/UI/OutputLogWindow.h"

#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    void OutputLogWindow::Draw()
    {
        ImGui::SetNextWindowSize(ImVec2(640.0f, 220.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(312.0f, 512.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("OutputLog", &m_isOpen);
        if (m_logSystem == nullptr)
        {
            ImGui::TextDisabled("No log system bound.");
        }
        else
        {
            ImGui::BeginChild("Text Output", ImVec2(0, 0), true);
            const std::string logContent = m_logSystem->GetLogContent();
            ImGui::TextUnformatted(logContent.c_str());
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void OutputLogWindow::BindLogSystem(Logging::LogSystem* logSystem)
    {
        m_logSystem = logSystem;
    }
}
