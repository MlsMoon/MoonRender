#pragma once

#include "Source/Object/MoonComponent.h"

namespace Object
{
    class CameraComponent final : public MoonComponent
    {
    public:
        ComponentType GetType() const override { return ComponentType::Camera; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Camera; }

        float fovRadians = 1.57079632679f;
        float nearPlane = 1.0f;
        float farPlane = 1000.0f;
    };
}
