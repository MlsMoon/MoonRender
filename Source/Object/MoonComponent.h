#pragma once

namespace Object
{
    enum class ComponentType
    {
        Transform,
        Mesh,
        Light,
        Camera
    };

    enum class ComponentConflictGroup
    {
        None,
        Transform,
        Renderable,
        Light,
        Camera
    };

    class MoonComponent
    {
    public:
        virtual ~MoonComponent() = default;

        virtual ComponentType GetType() const = 0;
        virtual ComponentConflictGroup GetConflictGroup() const { return ComponentConflictGroup::None; }
        virtual bool AllowMultiple() const { return false; }
    };
}
