#pragma once

#include <memory>
#include <vector>

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
            Object::MoonObject*& selectedObject);
        bool BindLogSystem(Logging::LogSystem* log_system);

    private:
        void DrawMainMenu();
        void DrawDockSpace();
        void DrawOutlineView(
            const std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects,
            Object::MoonObject*& selectedObject);
        void DrawInspectorView(Object::MoonObject* selectedObject);
        void DrawOutputLog();

    private:
        bool showOutlineWindow = true;
        bool showInspectorWindow = true;
        bool showOutputWindow = false;

        Logging::LogSystem* log_system = nullptr;
    };
}
