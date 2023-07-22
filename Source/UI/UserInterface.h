#pragma once

#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/EventSystem/EventCenter.h"
#include "Source/EventSystem/LogSystem.h"

namespace MoonUI
{
    class UserInterface
    {
    public:
        UserInterface();
        ~UserInterface();
        bool DrawMainInterfaceUI();
        bool BindLogSystem(const EventSystem::LogSystem* log_system);

    private:
        //Window Disable
        bool showCameraWindow = true;
        bool showOutputWindow = false;
        
        //Camera value
        float ui_camera_fov = 90.0f;

        EventSystem::LogSystem* log_system = nullptr;
    };
}
