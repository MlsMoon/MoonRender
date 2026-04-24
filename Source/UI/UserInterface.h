#pragma once

#include <memory>
#include <vector>

#include "Source/Graphics/public/GraphicsTypes.h"
#include "Source/Logging/public/LogSystem.h"
#include "Source/ThirdParty/ImGui/imgui.h"

namespace Object
{
    class MoonObject;
}

namespace MoonUI
{
    class UserInterface
    {
    public:
        UserInterface();
        ~UserInterface();
        bool DrawMainInterfaceUI(
            const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
            Object::MoonObject*& selectedObject,
            GraphicsBackendType graphicsBackendType);
        bool BindLogSystem(Logging::LogSystem* log_system);

    private:
        void DrawMainMenu();
        void DrawDockSpace();
        void DrawOutlineView(
            const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
            Object::MoonObject*& selectedObject);
        void DrawInspectorView(Object::MoonObject* selectedObject);
        void DrawGlobalSettingView(GraphicsBackendType graphicsBackendType);
        void DrawOutputLog();

    private:
        bool showOutlineWindow = true;
        bool showInspectorWindow = true;
        bool showGlobalSettingWindow = true;
        bool showOutputWindow = false;

        Logging::LogSystem* log_system = nullptr;
    };
}
