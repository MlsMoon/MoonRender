#pragma once

#include <algorithm>
#include <cmath>
#include <directxmath.h>

#include "Source/Object/MoonComponent.h"

namespace Object
{
    class CameraComponent final : public MoonComponent
    {
    public:
        static constexpr float MinFovDegrees = 10.0f;
        static constexpr float MaxFovDegrees = 120.0f;
        static constexpr float MinNearPlane = 0.001f;
        static constexpr float MinPlaneGap = 0.001f;
        static constexpr float MaxFarPlane = 10000.0f;

        ComponentType GetType() const override { return ComponentType::Camera; }
        const char* GetDisplayName() const override { return "Camera"; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Camera; }
        std::vector<ComponentProperty> GetProperties() override
        {
            return {
                {
                    "FOV",
                    ComponentPropertyType::Float,
                    false,
                    true,
                    0.1f,
                    MinFovDegrees,
                    MaxFovDegrees,
                    "%.1f deg",
                    {},
                    [this]() { return DirectX::XMConvertToDegrees(fovRadians); },
                    [this](float value)
                    {
                        fovRadians = DirectX::XMConvertToRadians(value);
                        Normalize();
                    },
                    {},
                    {},
                    {},
                    {}
                },
                {
                    "Near",
                    ComponentPropertyType::Float,
                    false,
                    true,
                    0.05f,
                    MinNearPlane,
                    MaxFarPlane - MinPlaneGap,
                    "%.3f",
                    {},
                    [this]() { return nearPlane; },
                    [this](float value)
                    {
                        nearPlane = value;
                        Normalize();
                    },
                    {},
                    {},
                    {},
                    {}
                },
                {
                    "Far",
                    ComponentPropertyType::Float,
                    false,
                    true,
                    1.0f,
                    MinNearPlane + MinPlaneGap,
                    MaxFarPlane,
                    "%.3f",
                    {},
                    [this]() { return farPlane; },
                    [this](float value)
                    {
                        farPlane = value;
                        Normalize();
                    },
                    {},
                    {},
                    {},
                    {}
                }
            };
        }

        void Normalize() override
        {
            float fovDegrees = DirectX::XMConvertToDegrees(fovRadians);
            if (!std::isfinite(fovDegrees))
            {
                fovDegrees = 90.0f;
            }
            fovDegrees = std::max(MinFovDegrees, std::min(fovDegrees, MaxFovDegrees));
            fovRadians = DirectX::XMConvertToRadians(fovDegrees);

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
