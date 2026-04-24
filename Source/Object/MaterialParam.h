#pragma once
#include <directxmath.h>

namespace Object
{
    struct MaterialParam
    {
        MaterialParam() = default;

        MaterialParam(const MaterialParam&) = default;
        MaterialParam& operator=(const MaterialParam&) = default;

        MaterialParam(MaterialParam&&) = default;
        MaterialParam& operator=(MaterialParam&&) = default;

        MaterialParam(const DirectX::XMFLOAT4& _ambient, const DirectX::XMFLOAT4& _diffuse, const DirectX::XMFLOAT4& _specular,
            const DirectX::XMFLOAT4& _reflect) :
            ambient(_ambient), diffuse(_diffuse), specular(_specular), reflect(_reflect) {}

        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 diffuse;
        DirectX::XMFLOAT4 specular; // w = specular intensity
        DirectX::XMFLOAT4 reflect;
    };
}
