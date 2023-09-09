#pragma once

#include "Source/Logging/public/LogSystem.h"
#include "Source/Scene/Public/MoonScene.h"

namespace MoonUI
{
    class UserInterface
    {
    public:
        UserInterface(MoonScene* scene);
        ~UserInterface();
        bool DrawMainInterfaceUI();

    private:
        //Window Disable
        bool showCameraWindow = true;
        bool showObjectDetailWindow = false;
        bool showOutputWindow = false;
        bool showSceneContent = false;
        
        MoonScene* moon_scene;
        
        //Camera value
        float ui_camera_fov = 90.0f;

        Logging::LogSystem* log_system = nullptr;
    };
}
