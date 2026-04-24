#include "Source/UI/Editor.h"

#include "Source/ThirdParty/ImGui/imgui.h"

namespace MoonUI
{
    bool Editor::Draw(
        std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
        Object::MoonObject*& selectedObject,
        GraphicsBackendType graphicsBackendType)
    {
        bool showOutline = m_outlineWindow.IsOpen();
        bool showInspector = m_inspectorWindow.IsOpen();
        bool showGlobalSetting = m_globalSettingWindow.IsOpen();
        bool showOutputLog = m_outputLogWindow.IsOpen();

        m_mainMenuBar.Draw(&showOutline, &showInspector, &showGlobalSetting, &showOutputLog);

        m_outlineWindow.SetOpen(showOutline);
        m_inspectorWindow.SetOpen(showInspector);
        m_globalSettingWindow.SetOpen(showGlobalSetting);
        m_outputLogWindow.SetOpen(showOutputLog);

        DrawDockSpace();

        if (m_outlineWindow.IsOpen())
        {
            m_outlineWindow.Draw(sceneObjects, selectedObject);
        }

        if (m_inspectorWindow.IsOpen())
        {
            m_inspectorWindow.Draw(selectedObject);
        }

        if (m_globalSettingWindow.IsOpen())
        {
            m_globalSettingWindow.Draw(graphicsBackendType);
        }

        if (m_outputLogWindow.IsOpen())
        {
            m_outputLogWindow.Draw();
        }

        return true;
    }

    void Editor::DrawDockSpace()
    {
        ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspaceFlags);
    }

    bool Editor::BindLogSystem(Logging::LogSystem* log_system)
    {
        m_outputLogWindow.BindLogSystem(log_system);
        return true;
    }

    Editor::Editor() = default;
    Editor::~Editor() = default;
}
