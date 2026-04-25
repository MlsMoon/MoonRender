#pragma once
#include "Source/Math/public/MoonMath.h"

namespace Object
{
    struct DirectionalLight
    {
        DirectionalLight() = default;

        DirectionalLight(const DirectionalLight&) = default;
        DirectionalLight& operator=(const DirectionalLight&) = default;

        DirectionalLight(DirectionalLight&&) = default;
        DirectionalLight& operator=(DirectionalLight&&) = default;

        DirectionalLight(const MoonVector4& _direction_intensity) : direction_intensity(_direction_intensity) {}

        MoonVector4 direction_intensity; // dir : x,y,z intensity: w
    };

    struct PointLight
    {
        PointLight() = default;

        PointLight(const PointLight&) = default;
        PointLight& operator=(const PointLight&) = default;

        PointLight(PointLight&&) = default;
        PointLight& operator=(PointLight&&) = default;

        PointLight(const MoonVector4& _ambient, const MoonVector4& _diffuse, const MoonVector4& _specular,
            const MoonVector3& _position, float _range, const MoonVector3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), position(_position), range(_range), att(_att), pad() {}

        MoonVector4 ambient;
        MoonVector4 diffuse;
        MoonVector4 specular;

        MoonVector3 position;
        float range;

        MoonVector3 att;
        float pad;
    };

    struct SpotLight
    {
        SpotLight() = default;

        SpotLight(const SpotLight&) = default;
        SpotLight& operator=(const SpotLight&) = default;

        SpotLight(SpotLight&&) = default;
        SpotLight& operator=(SpotLight&&) = default;

        SpotLight(const MoonVector4& _ambient, const MoonVector4& _diffuse, const MoonVector4& _specular,
            const MoonVector3& _position, float _range, const MoonVector3& _direction,
            float _spot, const MoonVector3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular),
            position(_position), range(_range), direction(_direction), spot(_spot), att(_att), pad() {}

        MoonVector4 ambient;
        MoonVector4 diffuse;
        MoonVector4 specular;

        MoonVector3 position;
        float range;

        MoonVector3 direction;
        float spot;

        MoonVector3 att;
        float pad;
    };
}
