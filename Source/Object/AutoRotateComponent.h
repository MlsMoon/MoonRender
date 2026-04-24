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
        ComponentType GetType() const override { return ComponentType::AutoRotate; }
        const char* GetDisplayName() const override { return "Auto Rotate"; }

        std::vector<ComponentProperty> GetProperties() override
        {
            return {
                {
                    "Angular Velocity",
                    ComponentPropertyType::Float3,
                    false,
                    false,
                    0.01f,
                    0.0f,
                    0.0f,
                    "%.3f rad/s",
                    {},
                    {},
                    {},
                    [this]() { return angularVelocityRadians; },
                    [this](const DirectX::XMFLOAT3& value) { angularVelocityRadians = value; },
                    {},
                    {}
                }
            };
        }

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
