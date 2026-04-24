#pragma once

#include "Source/Object/LightType.h"
#include "Source/Object/MoonComponent.h"

namespace Object
{
    enum class LightKind
    {
        Directional,
        Point,
        Spot
    };

    class LightComponent final : public MoonComponent
    {
    public:
        explicit LightComponent(LightKind lightKind = LightKind::Directional)
            : m_lightKind(lightKind)
        {
            directionalLight.direction_intensity = DirectX::XMFLOAT4(-0.577f, -0.577f, 0.577f, 1.0f);
        }

        ComponentType GetType() const override { return ComponentType::Light; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Light; }

        LightKind GetLightKind() const { return m_lightKind; }

        DirectionalLight directionalLight;
        PointLight pointLight;
        SpotLight spotLight;

    private:
        LightKind m_lightKind;
    };
}
