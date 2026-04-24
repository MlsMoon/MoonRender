#pragma once

#include <directxmath.h>

#include "Source/Object/MoonComponent.h"

namespace Object
{
    class TransformComponent final : public MoonComponent
    {
    public:
        ComponentType GetType() const override { return ComponentType::Transform; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Transform; }

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
