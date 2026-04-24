#pragma once

#include <memory>
#include <vector>

#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/UI/Popups/DeleteCameraWarningPopup.h"

namespace Object
{
    class MoonObject;
    class Scene;
}

namespace MoonUI
{
    class OutlineWindow
    {
    public:
        void Draw(Object::Scene& scene, Object::MoonObject*& selectedObject);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        void DrawObjectTree(Object::MoonObject* object, Object::MoonObject*& selectedObject, Object::Scene& scene);

    private:
        bool m_isOpen = true;
        DeleteCameraWarningPopup m_deleteCameraWarningPopup;
    };
}
