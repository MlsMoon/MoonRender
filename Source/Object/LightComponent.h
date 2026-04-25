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
            directionalLight.direction_intensity = glm::vec4(-0.577f, -0.577f, 0.577f, 1.0f);

            RegisterProperty(MoonProp::Text("Type",
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
                }));

            if (m_lightKind == LightKind::Directional)
            {
                RegisterProperty(MoonProp::Float4("Direction / Intensity", directionalLight.direction_intensity));
            }
        }

        MOON_COMPONENT(Light, "Light", Light)

        LightKind GetLightKind() const { return m_lightKind; }

        DirectionalLight directionalLight;
        PointLight pointLight;
        SpotLight spotLight;

    private:
        LightKind m_lightKind;
    };
}
