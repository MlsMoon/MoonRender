#pragma once

#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/EventSystem/EventCenter.h"

namespace MoonUI
{
    class UserInterface
    {
    public:
        bool DrawMainInterfaceUI();
        UserInterface();
        ~UserInterface();

    private:
        float ui_camera_fov = 90.0f;
    };
}
