#pragma once

namespace Object
{
    class MoonObject;
}

namespace MoonUI
{
    class InspectorWindow
    {
    public:
        void Draw(Object::MoonObject* selectedObject);

        bool IsOpen() const { return m_isOpen; }
        void SetOpen(bool open) { m_isOpen = open; }

    private:
        bool m_isOpen = true;
    };
}
