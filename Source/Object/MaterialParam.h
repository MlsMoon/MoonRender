#pragma once
#include "Source/Math/public/MoonMath.h"

namespace Object
{
    struct MaterialParam
    {
        MaterialParam() = default;

        MaterialParam(const MaterialParam&) = default;
        MaterialParam& operator=(const MaterialParam&) = default;

        MaterialParam(MaterialParam&&) = default;
        MaterialParam& operator=(MaterialParam&&) = default;

        MaterialParam(const MoonVector4& _ambient, const MoonVector4& _diffuse, const MoonVector4& _specular,
            const MoonVector4& _reflect) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), reflect(_reflect) {}

        MoonVector4 ambient;
        MoonVector4 diffuse;
        MoonVector4 specular; // w = specular intensity
        MoonVector4 reflect;
    };
}
