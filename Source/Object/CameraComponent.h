#pragma once

#include <algorithm>
#include <cmath>

#include "Source/Math/public/MoonMath.h"
#include "Source/Object/MoonComponent.h"

namespace Object
{
    class CameraComponent final : public MoonToggleableComponent
    {
    public:
        static constexpr float MinFovDegrees = 10.0f;
        static constexpr float MaxFovDegrees = 120.0f;
        static constexpr float MinNearPlane = 0.001f;
        static constexpr float MinPlaneGap = 0.001f;
        static constexpr float MaxFarPlane = 10000.0f;

        CameraComponent()
        {
            RegisterProperty(MoonProp::Float("FOV",
                [this]() { return MoonDegrees(fovRadians); },
                [this](float value)
                {
                    fovRadians = MoonRadians(value);
                    Normalize();
                },
                "%.1f deg", 0.1f, MinFovDegrees, MaxFovDegrees));
            RegisterProperty(MoonProp::Float("Near",
                [this]() { return nearPlane; },
                [this](float value)
                {
                    nearPlane = value;
                    Normalize();
                },
                "%.3f", 0.05f, MinNearPlane, MaxFarPlane - MinPlaneGap));
            RegisterProperty(MoonProp::Float("Far",
                [this]() { return farPlane; },
                [this](float value)
                {
                    farPlane = value;
                    Normalize();
                },
                "%.3f", 1.0f, MinNearPlane + MinPlaneGap, MaxFarPlane));
        }

        MOON_COMPONENT(Camera, "Camera", Camera)

        void Normalize() override
        {
            float fovDegrees = MoonDegrees(fovRadians);
            if (!std::isfinite(fovDegrees))
            {
                fovDegrees = 90.0f;
            }
            fovDegrees = std::max(MinFovDegrees, std::min(fovDegrees, MaxFovDegrees));
            fovRadians = MoonRadians(fovDegrees);

            if (!std::isfinite(nearPlane))
            {
                nearPlane = 1.0f;
            }
            if (!std::isfinite(farPlane))
            {
                farPlane = 1000.0f;
            }
            nearPlane = std::max(nearPlane, MinNearPlane);
            farPlane = std::max(farPlane, nearPlane + MinPlaneGap);
            farPlane = std::min(farPlane, MaxFarPlane);
            if (nearPlane >= farPlane)
            {
                nearPlane = std::max(MinNearPlane, farPlane - MinPlaneGap);
            }
        }

        float fovRadians = 1.57079632679f;
        float nearPlane = 1.0f;
        float farPlane = 1000.0f;
    };
}
