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
        const char* GetDisplayName() const override { return "Light"; }
        ComponentConflictGroup GetConflictGroup() const override { return ComponentConflictGroup::Light; }
        std::vector<ComponentProperty> GetProperties() override
        {
            std::vector<ComponentProperty> properties = {
                {
                    "Type",
                    ComponentPropertyType::Text,
                    true,
                    false,
                    0.0f,
                    0.0f,
                    0.0f,
                    "%s",
                    [this]()
                    {
                        switch (m_lightKind)
                        {
                        case LightKind::Point:
                            return std::string("Point");
                        case LightKind::Spot:
                            return std::string("Spot");
                        case LightKind::Directional:
                        default:
                            return std::string("Directional");
                        }
                    },
                    {},
                    {},
                    {},
                    {},
                    {},
                    {}
                }
            };

            if (m_lightKind == LightKind::Directional)
            {
                properties.push_back({
                    "Direction / Intensity",
                    ComponentPropertyType::Float4,
                    false,
                    false,
                    0.01f,
                    0.0f,
                    0.0f,
                    "%.3f",
                    {},
                    {},
                    {},
                    {},
                    {},
                    [this]() { return directionalLight.direction_intensity; },
                    [this](const DirectX::XMFLOAT4& value) { directionalLight.direction_intensity = value; }
                });
            }

            return properties;
        }

        LightKind GetLightKind() const { return m_lightKind; }

        DirectionalLight directionalLight;
        PointLight pointLight;
        SpotLight spotLight;

    private:
        LightKind m_lightKind;
    };
}
