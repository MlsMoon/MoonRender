#pragma once
#include "Source/ThirdParty/glm/glm.hpp"

namespace Object
{
    struct DirectionalLight
    {
        DirectionalLight() = default;

        DirectionalLight(const DirectionalLight&) = default;
        DirectionalLight& operator=(const DirectionalLight&) = default;

        DirectionalLight(DirectionalLight&&) = default;
        DirectionalLight& operator=(DirectionalLight&&) = default;

        DirectionalLight(const glm::vec4& _direction_intensity) : direction_intensity(_direction_intensity) {}

        glm::vec4 direction_intensity; // dir : x,y,z intensity: w
    };

    struct PointLight
    {
        PointLight() = default;

        PointLight(const PointLight&) = default;
        PointLight& operator=(const PointLight&) = default;

        PointLight(PointLight&&) = default;
        PointLight& operator=(PointLight&&) = default;

        PointLight(const glm::vec4& _ambient, const glm::vec4& _diffuse, const glm::vec4& _specular,
            const glm::vec3& _position, float _range, const glm::vec3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), position(_position), range(_range), att(_att), pad() {}

        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;

        glm::vec3 position;
        float range;

        glm::vec3 att;
        float pad;
    };

    struct SpotLight
    {
        SpotLight() = default;

        SpotLight(const SpotLight&) = default;
        SpotLight& operator=(const SpotLight&) = default;

        SpotLight(SpotLight&&) = default;
        SpotLight& operator=(SpotLight&&) = default;

        SpotLight(const glm::vec4& _ambient, const glm::vec4& _diffuse, const glm::vec4& _specular,
            const glm::vec3& _position, float _range, const glm::vec3& _direction,
            float _spot, const glm::vec3& _att) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular),
            position(_position), range(_range), direction(_direction), spot(_spot), att(_att), pad() {}

        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;

        glm::vec3 position;
        float range;

        glm::vec3 direction;
        float spot;

        glm::vec3 att;
        float pad;
    };
}
