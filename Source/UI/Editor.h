#pragma once

#include <memory>
#include <vector>

#include "Source/Graphics/public/GraphicsTypes.h"
#include "Source/Logging/public/LogSystem.h"
#include "Source/UI/GlobalSettingWindow.h"
#include "Source/UI/InspectorWindow.h"
#include "Source/UI/MainMenuBar.h"
#include "Source/UI/OutlineWindow.h"
#include "Source/UI/OutputLogWindow.h"
#include "Source/UI/ViewportWindow.h"

namespace Object
{
    class Scene;
}

namespace MoonUI
{
    class Editor
    {
    public:
        Editor();
        ~Editor();

        bool Draw(
            Object::Scene& scene,
            Object::MoonObject*& selectedObject,
            GraphicsBackendType graphicsBackendType,
            void* viewportTextureId,
            ViewportInfo& outViewportInfo);

        bool BindLogSystem(Logging::LogSystem* log_system);

    private:
        void DrawDockSpace();

        MainMenuBar m_mainMenuBar;
        OutlineWindow m_outlineWindow;
        InspectorWindow m_inspectorWindow;
        GlobalSettingWindow m_globalSettingWindow;
        OutputLogWindow m_outputLogWindow;
        ViewportWindow m_viewportWindow;
    };
}
