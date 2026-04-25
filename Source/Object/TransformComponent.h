#pragma once

#include "Source/ThirdParty/glm/glm.hpp"
#include "Source/ThirdParty/glm/gtc/matrix_transform.hpp"

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
                    return glm::vec3(
                        glm::degrees(rotationRadians.x),
                        glm::degrees(rotationRadians.y),
                        glm::degrees(rotationRadians.z));
                },
                [this](const glm::vec3& value)
                {
                    rotationRadians.x = glm::radians(value.x);
                    rotationRadians.y = glm::radians(value.y);
                    rotationRadians.z = glm::radians(value.z);
                },
                "%.2f deg", 0.25f));
            RegisterProperty(MoonProp::Float3("Scale", scale, "%.3f", 0.01f, 0.001f, 100.0f));
        }

        MOON_COMPONENT(Transform, "Transform", Transform)

        glm::mat4 GetLocalMatrix() const
        {
            const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
            const glm::mat4 rotationMatrix =
                glm::rotate(glm::mat4(1.0f), rotationRadians.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), rotationRadians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), rotationRadians.z, glm::vec3(0.0f, 0.0f, 1.0f));
            const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
            return scaleMatrix * rotationMatrix * translationMatrix;
        }

        glm::mat4 GetWorldMatrix() const
        {
            const glm::mat4 local = GetLocalMatrix();
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

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 rotationRadians = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    };
}
