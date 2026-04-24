#pragma once

#include <directxmath.h>

#include "Source/Object/MoonComponent.h"
#include "Source/Object/MoonObject.h"
#include "Source/Object/TransformComponent.h"

namespace Object
{
    class AutoRotateComponent final : public MoonComponent
    {
    public:
        AutoRotateComponent()
        {
            RegisterProperty(MoonProp::Float3("Angular Velocity", angularVelocityRadians, "%.3f rad/s", 0.01f));
        }

        MOON_COMPONENT(AutoRotate, "Auto Rotate", None)

        void Update(MoonObject& owner, float dt) override
        {
            TransformComponent* transform = owner.GetComponent<TransformComponent>();
            if (transform == nullptr)
            {
                return;
            }

            transform->rotationRadians.x += angularVelocityRadians.x * dt;
            transform->rotationRadians.y += angularVelocityRadians.y * dt;
            transform->rotationRadians.z += angularVelocityRadians.z * dt;
        }

        DirectX::XMFLOAT3 angularVelocityRadians = DirectX::XMFLOAT3(0.3f, 0.37f, 0.0f);
    };
}
