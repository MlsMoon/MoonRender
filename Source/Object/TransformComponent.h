#pragma once

#include <directxmath.h>

#include "Source/Object/MoonComponent.h"

namespace Object
{
    class TransformComponent final : public MoonComponent
    {
    public:
        ComponentType GetType() const override { return ComponentType::Transform; }
        const char* GetDisplayName() const override { return "Transform"; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Transform; }
        std::vector<ComponentProperty> GetProperties() override
        {
            return {
                {
                    "Position",
                    ComponentPropertyType::Float3,
                    false,
                    false,
                    0.05f,
                    0.0f,
                    0.0f,
                    "%.3f",
                    {},
                    {},
                    {},
                    [this]() { return position; },
                    [this](const DirectX::XMFLOAT3& value) { position = value; },
                    {},
                    {}
                },
                {
                    "Rotation",
                    ComponentPropertyType::Float3,
                    false,
                    false,
                    0.25f,
                    0.0f,
                    0.0f,
                    "%.2f deg",
                    {},
                    {},
                    {},
                    [this]()
                    {
                        return DirectX::XMFLOAT3(
                            DirectX::XMConvertToDegrees(rotationRadians.x),
                            DirectX::XMConvertToDegrees(rotationRadians.y),
                            DirectX::XMConvertToDegrees(rotationRadians.z));
                    },
                    [this](const DirectX::XMFLOAT3& value)
                    {
                        rotationRadians.x = DirectX::XMConvertToRadians(value.x);
                        rotationRadians.y = DirectX::XMConvertToRadians(value.y);
                        rotationRadians.z = DirectX::XMConvertToRadians(value.z);
                    },
                    {},
                    {}
                },
                {
                    "Scale",
                    ComponentPropertyType::Float3,
                    false,
                    true,
                    0.01f,
                    0.001f,
                    100.0f,
                    "%.3f",
                    {},
                    {},
                    {},
                    [this]() { return scale; },
                    [this](const DirectX::XMFLOAT3& value) { scale = value; },
                    {},
                    {}
                }
            };
        }

        DirectX::XMMATRIX GetWorldMatrix() const
        {
            const DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            const DirectX::XMMATRIX rotationMatrix =
                DirectX::XMMatrixRotationX(rotationRadians.x) *
                DirectX::XMMatrixRotationY(rotationRadians.y) *
                DirectX::XMMatrixRotationZ(rotationRadians.z);
            const DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
            return scaleMatrix * rotationMatrix * translationMatrix;
        }

        DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        DirectX::XMFLOAT3 rotationRadians = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    };
}
