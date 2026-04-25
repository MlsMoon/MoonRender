#pragma once

#include "Source/Math/public/MoonMath.h"
#include "Source/Object/MoonComponent.h"

namespace Object
{
    class TransformComponent final : public MoonComponent
    {
    public:
        TransformComponent()
        {
            RegisterProperty(MoonProp::Float3("Position", position, "%.3f", 0.05f));
            RegisterProperty(MoonProp::Float3("Rotation",
                [this]()
                {
                    return MoonVector3(
                        MoonDegrees(rotationRadians.x),
                        MoonDegrees(rotationRadians.y),
                        MoonDegrees(rotationRadians.z));
                },
                [this](const MoonVector3& value)
                {
                    rotationRadians.x = MoonRadians(value.x);
                    rotationRadians.y = MoonRadians(value.y);
                    rotationRadians.z = MoonRadians(value.z);
                },
                "%.2f deg", 0.25f));
            RegisterProperty(MoonProp::Float3("Scale", scale, "%.3f", 0.01f, 0.001f, 100.0f));
        }

        MOON_COMPONENT(Transform, "Transform", Transform)

        MoonMatrix4x4 GetLocalMatrix() const
        {
            const MoonMatrix4x4 scaleMatrix = MoonScale(scale);
            const MoonMatrix4x4 rotationMatrix =
                MoonRotate(rotationRadians.x, MoonVector3(1.0f, 0.0f, 0.0f)) *
                MoonRotate(rotationRadians.y, MoonVector3(0.0f, 1.0f, 0.0f)) *
                MoonRotate(rotationRadians.z, MoonVector3(0.0f, 0.0f, 1.0f));
            const MoonMatrix4x4 translationMatrix = MoonTranslate(position);
            return scaleMatrix * rotationMatrix * translationMatrix;
        }

        MoonMatrix4x4 GetWorldMatrix() const
        {
            const MoonMatrix4x4 local = GetLocalMatrix();
            if (GetOwner() != nullptr && GetOwner()->GetParent() != nullptr)
            {
                const auto* parentTransform = GetOwner()->GetParent()->GetComponent<TransformComponent>();
                if (parentTransform != nullptr)
                {
                    return local * parentTransform->GetWorldMatrix();
                }
            }
            return local;
        }

        MoonVector3 position = MoonVector3(0.0f, 0.0f, 0.0f);
        MoonVector3 rotationRadians = MoonVector3(0.0f, 0.0f, 0.0f);
        MoonVector3 scale = MoonVector3(1.0f, 1.0f, 1.0f);
    };
}
