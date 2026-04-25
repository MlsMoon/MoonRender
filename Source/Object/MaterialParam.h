#pragma once
#include "Source/ThirdParty/glm/glm.hpp"

namespace Object
{
    struct MaterialParam
    {
        MaterialParam() = default;

        MaterialParam(const MaterialParam&) = default;
        MaterialParam& operator=(const MaterialParam&) = default;

        MaterialParam(MaterialParam&&) = default;
        MaterialParam& operator=(MaterialParam&&) = default;

        MaterialParam(const glm::vec4& _ambient, const glm::vec4& _diffuse, const glm::vec4& _specular,
            const glm::vec4& _reflect) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), reflect(_reflect) {}

        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular; // w = specular intensity
        glm::vec4 reflect;
    };
}
