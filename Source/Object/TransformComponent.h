#pragma once

#include <directxmath.h>

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
                "%.2f deg", 0.25f));
            RegisterProperty(MoonProp::Float3("Scale", scale, "%.3f", 0.01f, 0.001f, 100.0f));
        }

        MOON_COMPONENT(Transform, "Transform", Transform)

        DirectX::XMMATRIX GetLocalMatrix() const
        {
            const DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            const DirectX::XMMATRIX rotationMatrix =
                DirectX::XMMatrixRotationX(rotationRadians.x) *
                DirectX::XMMatrixRotationY(rotationRadians.y) *
                DirectX::XMMatrixRotationZ(rotationRadians.z);
            const DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
            return scaleMatrix * rotationMatrix * translationMatrix;
        }

        DirectX::XMMATRIX GetWorldMatrix() const
        {
            const DirectX::XMMATRIX local = GetLocalMatrix();
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

        DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        DirectX::XMFLOAT3 rotationRadians = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    };
}
