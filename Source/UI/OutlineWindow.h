#pragma once

#include <memory>
#include <vector>

#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/UI/Popups/DeleteCameraWarningPopup.h"

namespace Object
{
    class MoonObject;
}

namespace MoonUI
{
    class OutlineWindow
    {
    public:
        void Draw(std::vector<std::unique_ptr<Object::MoonObject>>& sceneObjects, Object::MoonObject*& selectedObject);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = true;
        DeleteCameraWarningPopup m_deleteCameraWarningPopup;
    };
}
